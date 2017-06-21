#include "IbConnectionManager.h"

#include "ibnet/sys/Logger.hpp"

#include "IbTimeoutException.h"

#define PAKET_MAGIC 0xBEEFCA4E

namespace ibnet {
namespace core {

const uint32_t IbConnectionManager::CONNECT_RETRY_WAIT_MS = 20;

IbConnectionManager::IbConnectionManager(
        uint16_t ownNodeId,
        uint16_t socketPort,
        uint32_t maxNumConnections,
        std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom,
        std::shared_ptr<IbDiscoveryManager>& discoveryManager,
        std::unique_ptr<IbConnectionCreator> connectionCreator) :
    ThreadLoop("ConnectionManager"),
    m_socket(socketPort),
    m_maxNumConnections(maxNumConnections),
    m_device(device),
    m_protDom(protDom),
    m_discoveryManager(discoveryManager),
    m_connectionCreator(std::move(connectionCreator)),
    m_ownNodeId(ownNodeId),
    m_openConnections(0),
    m_listener(nullptr),
    m_buffer(nullptr)
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("Initializing connection manager, node 0x{:x}", ownNodeId);

    if (ownNodeId == IbNodeId::INVALID) {
        throw IbException("Invalid node id provided");
    }

    // fill array with 'unique' connection ids
    // ids are reused to ensure the max id is max_connections - 1
    for (int i = m_maxNumConnections - 1; i >= 0; i--) {
        uint16_t tmp = (uint16_t) i;
        m_availableConnectionIds.push_back(tmp);
    }

    Start();
}

IbConnectionManager::~IbConnectionManager(void)
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("Shutting down connection manager...");

    Stop();

    // close opened connections
    for (uint32_t i = 0; i < IbNodeId::MAX_NUM_NODES; i++) {
        if (m_connections[i]) {
            m_connections[i]->Close(true);
            m_connections[i].reset();
        }
    }

    m_connectionCreator.reset();

    IBNET_LOG_DEBUG("Shutting down connection manager done");
}

std::shared_ptr<IbConnection> IbConnectionManager::GetConnection(uint16_t nodeId)
{
    IBNET_LOG_TRACE("GetConnection: 0x{:x}", nodeId);

    if (nodeId == IbNodeId::INVALID) {
        throw IbException("Invalid node id provided");
    }

    // connection already established?
    std::shared_ptr<IbConnection> connection = m_connections[nodeId];
    if (connection) {
        IBNET_LOG_TRACE("GetConnection (available): 0x{:x}", nodeId);
        return connection;
    }

    m_connectionMutex.lock();

    IBNET_LOG_TRACE("GetConnection (active create): 0x{:x}", nodeId);

    std::shared_ptr<IbNodeConf::Entry> nodeInfo;
    try {
        // get node connection information
        nodeInfo = m_discoveryManager->GetNodeInfo(nodeId);
    } catch (...) {
        m_connectionMutex.unlock();
        throw;
    }

    // check first if creating a connection is already in progress
    if (!m_connections[nodeId]) {
        IBNET_LOG_DEBUG("Establish connection request to 0x{:x}", nodeId);

        uint16_t connectionId = m_availableConnectionIds.back();
        m_availableConnectionIds.pop_back();

        m_connections[nodeId] = m_connectionCreator->CreateConnection(
            connectionId, m_device, m_protDom);

        for (auto& it : m_connections[nodeId]->GetQps()) {
            m_qpNumToNodeIdMappings.insert(std::make_pair(
                it->GetPhysicalQpNum(), nodeId));
        }

        m_connectionMutex.unlock();

        if (m_connections[nodeId]->GetQps().size() > MAX_QPS_PER_CONNECTION) {
            throw IbException("Exceeded max qps per connection limit");
        }

        // send own qp info to remote to trigger remote qp creation
        PaketNodeInfo paket;

        paket.m_magic = PAKET_MAGIC;
        paket.m_info.m_nodeId = m_ownNodeId;
        paket.m_info.m_lid = m_device->GetLid();
        memset(paket.m_info.m_physicalQpId, 0xFF,
            sizeof(uint32_t) * MAX_QPS_PER_CONNECTION);

        uint32_t cnt = 0;
        for (auto& it : m_connections[nodeId]->GetQps()) {
            paket.m_info.m_physicalQpId[cnt++] = it->GetPhysicalQpNum();
        }

        uint32_t tryCounter = 0;

        while (!m_connections[nodeId]->IsConnected()) {
            IBNET_LOG_TRACE("Sending connection exchg information to {}",
                    nodeInfo->GetAddress());
            ssize_t ret = m_socket.Send(&paket, sizeof(PaketNodeInfo),
                    nodeInfo->GetAddress().GetAddress(), m_socket.GetPort());

            if (ret != sizeof(PaketNodeInfo)) {
                IBNET_LOG_ERROR(
                        "Trying to establish connection to node 0x{:x} failed,"
                        " sending initial UDP paket failed",
                        nodeId);
            }
            if (tryCounter > MAX_CONNECT_RETIRES) {

                m_connectionMutex.lock();

                // one last check
                if (m_connections[nodeId]->IsConnected()) {
                    m_connectionMutex.unlock();
                    // last minute success
                    return m_connections[nodeId];
                }

                // FIXME don't remove because entries are replaced on new connection anyway? plus avoid race condition?
//                for (auto& it : m_connections[nodeId]->GetQps()) {
//                    m_qpNumToNodeIdMappings.erase(it->GetPhysicalQpNum());
//                }

                m_connections[nodeId].reset();

                m_connectionMutex.unlock();

                m_discoveryManager->InvalidateNodeInfo(nodeId);

                throw IbTimeoutException(nodeId, "Connection retry count exceeded");
            }

            tryCounter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_RETRY_WAIT_MS));
        }
    } else {
        m_connectionMutex.unlock();

        // wait until we received the remote info and established the connection
        while (!m_connections[nodeId]->IsConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    return m_connections[nodeId];
}

void IbConnectionManager::CloseConnection(uint16_t nodeId, bool force)
{
    IBNET_LOG_INFO("Closing connection of 0x{:x}, force {}", nodeId, force);

    m_connectionMutex.lock();

    // remove connection
    std::shared_ptr<core::IbConnection> connection = m_connections[nodeId];
    m_connections[nodeId] = nullptr;

    // check if someone else was faster and removed it already
    if (connection == nullptr) {
        m_connectionMutex.unlock();
        return;
    }

    // wait until we own the only reference to the object
    // let's hope nobody is caching/storing one
    while (!connection.unique()) {
        std::this_thread::yield();
    }

    // FIXME don't remove because entries are replaced on new connection anyway? plus avoid race condition?
//    for (auto& it : m_connections[nodeId]->GetQps()) {
//        m_qpNumToNodeIdMappings.erase(it->GetPhysicalQpNum());
//    }

    connection->Close(force);

    // re-use connection id
    m_availableConnectionIds.push_back(connection->GetConnectionId());

    connection.reset();

    m_openConnections--;

    m_connectionMutex.unlock();

    m_discoveryManager->InvalidateNodeInfo(nodeId);

    if (m_listener) {
        m_listener->NodeDisconnected(nodeId);
    }

    IBNET_LOG_DEBUG("Connection of 0x{:x}, force {} closed", nodeId, force);
}

void IbConnectionManager::_BeforeRunLoop(void)
{
    m_buffer = malloc(sizeof(PaketNodeInfo));
}

void IbConnectionManager::_RunLoop(void)
{
    const size_t bufferSize = sizeof(PaketNodeInfo);
    uint32_t recvAddr = 0;

    ssize_t res = m_socket.Receive(m_buffer, bufferSize, &recvAddr);

    if (res == bufferSize) {
        PaketNodeInfo* paket = (PaketNodeInfo*) m_buffer;

        IBNET_LOG_TRACE("Received paket from {}, magic 0x{:x}",
                sys::AddressIPV4(recvAddr), paket->m_magic);

        if (paket->m_magic == PAKET_MAGIC) {

            uint16_t remoteNodeId = paket->m_info.m_nodeId;
            IBNET_LOG_DEBUG("Received establish connection request by 0x{:x}",
                    remoteNodeId);

            m_connectionMutex.lock();

            // check first if creating a connection is already in progress
            if (m_connections[remoteNodeId] == nullptr) {
                IBNET_LOG_TRACE("Creating new connection for node 0x{:X}",
                        remoteNodeId);

                uint16_t connectionId = m_availableConnectionIds.back();
                m_availableConnectionIds.pop_back();

                // create new connection for remote request
                m_connections[remoteNodeId] =
                    m_connectionCreator->CreateConnection(connectionId,
                        m_device, m_protDom);

                for (auto& it : m_connections[remoteNodeId]->GetQps()) {
                    m_qpNumToNodeIdMappings.insert(std::make_pair(
                        it->GetPhysicalQpNum(), remoteNodeId));
                }
            }

            // check if the QP is already connected
            if (!m_connections[remoteNodeId]->IsConnected()) {
                std::vector<uint32_t> remotePhysicalQpIds;
                for (uint32_t i = 0; i < MAX_QPS_PER_CONNECTION; i++) {
                    if (paket->m_info.m_physicalQpId[i] == 0xFFFFFFFF) {
                        break;
                    }

                    remotePhysicalQpIds.push_back(
                        paket->m_info.m_physicalQpId[i]);
                }

                IbRemoteInfo remoteInfo(paket->m_info.m_nodeId,
                    paket->m_info.m_lid, remotePhysicalQpIds);

                m_connections[remoteNodeId]->Connect(remoteInfo);
                IBNET_LOG_INFO("Connected QP to remote {}", remoteInfo);
                m_openConnections++;

                m_connectionMutex.unlock();

                // listener _after_ unlock -> listener can call GetConnection
                // => deadlock
                if (m_listener) {
                    m_listener->NodeConnected(remoteNodeId,
                            *m_connections[remoteNodeId]);
                }

                // other QP sent request and is already connected,
                // no need to reply

                return;
            }
            // else: QP already connected concurrently by other
            // thread

            // reply with connection information to remote
            paket->m_info.m_nodeId = m_ownNodeId;
            paket->m_info.m_lid = m_device->GetLid();

            uint32_t cnt = 0;
            for (auto& it : m_connections[remoteNodeId]->GetQps()) {
                paket->m_info.m_physicalQpId[cnt++] = it->GetPhysicalQpNum();
            }

            m_connectionMutex.unlock();

            IBNET_LOG_TRACE("Replying with connection exchg info to node 0x{:X}",
                    remoteNodeId);
            res = m_socket.Send(m_buffer, bufferSize, recvAddr, m_socket.GetPort());

            if (res != bufferSize) {
                IBNET_LOG_ERROR("Establishing connection with node 0x{:x}"
                        " failed, sending response failed", remoteNodeId);
            }
        } else {
            IBNET_LOG_TRACE("Received paket with invalid magic 0x{:x}",
                    paket->m_magic);
        }
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

void IbConnectionManager::_AfterRunLoop(void)
{
    free(m_buffer);
}

}
}