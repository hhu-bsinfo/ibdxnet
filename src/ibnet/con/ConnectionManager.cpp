//
// Created by nothaas on 1/29/18.
//

#include "ConnectionManager.h"

#include "ibnet/sys/IllegalStateException.h"
#include "ibnet/sys/Logger.hpp"
#include "ibnet/sys/Random.h"
#include "ibnet/sys/TimeoutException.h"

#include "ibnet/core/IbException.h"

#include "InvalidNodeIdException.h"

namespace ibnet {
namespace con {

ConnectionManager::ConnectionManager(const std::string& name,
        NodeId ownNodeId, const NodeConf& nodeConf,
        uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
        core::IbDevice* refDevice, core::IbProtDom* refProtDom,
        ExchangeManager* refExchangeManager, JobManager* refJobManager,
        DiscoveryManager* refDiscoveryManager) :
    m_name(name),
    m_ownNodeId(ownNodeId),
    m_connectionCreationTimeoutMs(connectionCreationTimeoutMs),
    m_maxNumConnections(maxNumConnections),
    m_connectionCtxIdent(sys::Random::Generate32()),
    m_refDevice(refDevice),
    m_refProtDom(refProtDom),
    m_refExchangeManager(refExchangeManager),
    m_refJobManager(refJobManager),
    m_refDiscoveryManager(refDiscoveryManager),
    m_listener(nullptr),
    m_connectionStates(),
    m_connections(),
    m_openConnections(0),
    m_availableConnectionIds(),
    m_flagShutdown(false),
    m_conDataExchgPaketType(m_refExchangeManager->GeneratePaketTypeId()),
    m_createConnectionJobType(m_refJobManager->GenerateJobTypeId()),
    m_connectConnectionJobType(m_refJobManager->GenerateJobTypeId()),
    m_closeConnectionJobType(m_refJobManager->GenerateJobTypeId())
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("[%s] Initializing connection manager, ownNodeId %X, "
        "creation timeout %d, max connections %d, node config %s", m_name,
        ownNodeId, connectionCreationTimeoutMs, maxNumConnections, nodeConf);

    if (ownNodeId == NODE_ID_INVALID) {
        throw sys::IllegalStateException("Invalid node id provided");
    }

    for (uint32_t i = 0; i < NODE_ID_MAX_NUM_NODES; i++) {
        m_connections[i] = nullptr;
    }

    // fill array with 'unique' connection ids
    // ids are reused to ensure the max id is max_connections - 1
    for (int i = m_maxNumConnections - 1; i >= 0; i--) {
        m_availableConnectionIds.push_back((uint16_t) i);
    }

    m_refExchangeManager->AddDispatcher(m_conDataExchgPaketType, this);

    m_refJobManager->AddDispatcher(m_createConnectionJobType, this);
    m_refJobManager->AddDispatcher(m_connectConnectionJobType, this);
    m_refJobManager->AddDispatcher(m_closeConnectionJobType, this);

    IBNET_LOG_DEBUG("[%s] Initializing connection manager done", m_name);
}

ConnectionManager::~ConnectionManager()
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("[%s] Shutting down connection manager...", m_name);

    m_flagShutdown.store(true, std::memory_order_relaxed);

    // close opened connections
    for (auto& m_connection : m_connections) {
        if (m_connection) {
            __AddJobCloseConnection(m_connection->GetRemoteNodeId(),
                true, true);
        }
    }

    // wait until all jobs are processed
    while (!m_refJobManager->IsQueueEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    m_refExchangeManager->RemoveDispatcher(m_conDataExchgPaketType, this);

    m_refJobManager->RemoveDispatcher(m_createConnectionJobType, this);
    m_refJobManager->RemoveDispatcher(m_connectConnectionJobType, this);
    m_refJobManager->RemoveDispatcher(m_closeConnectionJobType, this);

    IBNET_LOG_DEBUG("[%s] Shutting down connection manager done", m_name);
}

Connection* ConnectionManager::GetConnection(NodeId nodeId)
{
    if (nodeId == NODE_ID_INVALID) {
        throw InvalidNodeIdException();
    }

    if (nodeId == m_ownNodeId) {
        throw InvalidNodeIdException(nodeId,
            "[%s] MsgrcLoopbackSystem connection to own node not allowed",
            m_name);
    }

    // keep track of "handles" issued
    int32_t available =
        m_connectionStates[nodeId].m_available.load(std::memory_order_acquire);

    if (available >= ConnectionState::CONNECTION_AVAILABLE) {
        return m_connections[nodeId];
    }

    IBNET_LOG_TRACE("[%s] GetConnection: 0x%X, avail: %d", m_name, nodeId,
        available + 1);

    // This is used to trigger periodic re-transmits for the connection exchg
    // data. Only the first thread triggering the creation job has to do
    // this for the current connection getting created
    bool triggerPeriodicRecreation = false;
    uint8_t state = ConnectionState::e_StateNotAvailable;

    // avoid flooding the job queue with creation jobs, only the first thread
    // has to add the job
    if (m_connectionStates[nodeId].m_state.compare_exchange_strong(state,
            ConnectionState::e_StateInCreation, std::memory_order_relaxed)) {
        triggerPeriodicRecreation = true;
        __AddJobCreateConnection(nodeId);
    }

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point startRetry;
    std::chrono::high_resolution_clock::time_point end;

    start = std::chrono::high_resolution_clock::now();
    startRetry = start;

    do {
        available = m_connectionStates[nodeId].m_available
            .load(std::memory_order_acquire);

        if (available >= ConnectionState::CONNECTION_AVAILABLE) {
            m_connectionStates[nodeId].m_available
                .fetch_add(1, std::memory_order_acquire);

            // sanity check
            if (m_connections[nodeId] == nullptr) {
                throw sys::IllegalStateException(
                    "[%s] Invalid connection state on GetConnection",
                    m_name);
            }

            return m_connections[nodeId];
        }

        std::this_thread::yield();

        end = std::chrono::high_resolution_clock::now();

        // to handle package loss on UDP sockets, if the remote didn't receive
        // any exchange data so far, ensure it is resent ever second at least
        // the connection will not be allocated again but the connection
        // exchg data is sent again with that job
        if (triggerPeriodicRecreation) {
            if (end - startRetry >= std::chrono::milliseconds(1000)) {
                if (m_connections[nodeId] &&
                        !(m_connectionStates[nodeId].m_exchgFlags
                        .load(std::memory_order_relaxed) &
                        ConnectionState::e_ExchgFlagRemoteConnected)) {
                    __AddJobCreateConnection(nodeId);
                }

                startRetry = end;
            }
        }
    } while (end - start <
             std::chrono::milliseconds(m_connectionCreationTimeoutMs));


    std::chrono::duration<uint64_t, std::nano> delta(end - start);
    throw sys::TimeoutException(
        "[%s] Creating connection connection to %X, timeout: %d ms", m_name,
        nodeId, delta.count() / 1000 / 1000);
}

void ConnectionManager::ReturnConnection(Connection* connection)
{
    m_connectionStates[connection->GetRemoteNodeId()].m_available.fetch_sub(1,
        std::memory_order_relaxed);
}

void ConnectionManager::CloseConnection(NodeId nodeId, bool force)
{
    __AddJobCloseConnection(nodeId, force, false);
}

void ConnectionManager::_DispatchExchangeData(uint32_t sourceIPV4,
        const ExchangeManager::PaketHeader* paketHeader,
        const void* data)
{
    if (paketHeader->m_type == m_conDataExchgPaketType) {
        auto* connectionHeader =
            static_cast<const RemoteConnectionHeader*>(data);
        auto* connectionData = (const void*)
            (((const uintptr_t ) data) + sizeof(RemoteConnectionHeader));
        size_t connectionDataSize =
            paketHeader->m_length - sizeof(RemoteConnectionHeader);

        if (paketHeader->m_length < sizeof(RemoteConnectionHeader)) {
            throw sys::IllegalStateException();
        }

        // create a copy of the data, don't use receive buffer pointer
        auto* copyData = new uint8_t[connectionDataSize];
        memcpy(copyData, connectionData, connectionDataSize);

        __AddJobConnectConnection(*connectionHeader, copyData,
            connectionDataSize);
    }
}

void ConnectionManager::_DispatchJob(const JobQueue::Job* job)
{
    if (job->m_type == m_createConnectionJobType) {
        // don't create new connections when shutting down
        if (!m_flagShutdown.load(std::memory_order_relaxed)) {
            __JobDispatchCreateConnection(
                *dynamic_cast<const JobCreateConnection*>(job));
        }
    } else if (job->m_type == m_connectConnectionJobType) {
        // don't connect connections when shutting down
        if (!m_flagShutdown.load(std::memory_order_relaxed)) {
            __JobDispatchConnectConnection(
                *dynamic_cast<const JobConnectConnection*>(job));
        }
    } else if (job->m_type == m_closeConnectionJobType) {
        __JobDispatchCloseConnection(
            *dynamic_cast<const JobCloseConnection*>(job));
    }
}

void ConnectionManager::__AddJobCreateConnection(NodeId destNodeId)
{
    m_refJobManager->AddJob(new JobCreateConnection(m_createConnectionJobType,
        destNodeId));
}

void ConnectionManager::__AddJobConnectConnection(
        const RemoteConnectionHeader& remoteConnectionHeader,
        const uint8_t* remoteConnectionData, size_t remoteConnectionDatSize)
{
    m_refJobManager->AddJob(new JobConnectConnection(m_connectConnectionJobType,
        remoteConnectionHeader, remoteConnectionData,
        remoteConnectionDatSize));
}

void ConnectionManager::__AddJobCloseConnection(NodeId nodeId, bool force,
        bool shutdown)
{
    m_refJobManager->AddJob(new JobCloseConnection(m_closeConnectionJobType,
        nodeId, force, shutdown));
}

void ConnectionManager::__JobDispatchCreateConnection(
        const JobCreateConnection& job)
{
    NodeConf::Entry discoveryRemoteNodeInfo;

    // only create connection if not created or in creation
    if (m_connectionStates[job.m_targetNodeId].m_state
            .load(std::memory_order_relaxed) ==
            ConnectionState::e_StateInCreation) {
        IBNET_LOG_DEBUG("[%s] Create connection, target node id 0x%X",
            m_name, job.m_targetNodeId);

        try {
            // try to get remote node connection info from discovery man to
            // check if already discovered
            discoveryRemoteNodeInfo = m_refDiscoveryManager->GetNodeInfo(
                job.m_targetNodeId);
        } catch (...) {
            IBNET_LOG_WARN("[%s] Cannot create connection to remote 0x%X, "
                "not discovered, yet", m_name, job.m_targetNodeId);
            return;
        }

        __AllocateConnection(job.m_targetNodeId);
    }

    __ReplyConnectionExchgData(job.m_targetNodeId,
        m_connectionStates[job.m_targetNodeId].m_exchgFlags
            .load(std::memory_order_relaxed),
        discoveryRemoteNodeInfo.GetAddress().GetAddress());
}

void ConnectionManager::__JobDispatchConnectConnection(
        const JobConnectConnection& job)
{
    IBNET_LOG_TRACE_FUNC;

    NodeConf::Entry discoveryRemoteNodeInfo;

    IBNET_LOG_DEBUG("[%s] Connect connection job, target node id 0x%X, con "
        "state 0x%X, lid 0x%X, con man ident 0x%X, data size %d", m_name,
        job.m_remoteConnectionHeader.m_nodeId,
        job.m_remoteConnectionHeader.m_connectionState,
        job.m_remoteConnectionHeader.m_lid,
        job.m_remoteConnectionHeader.m_conManIdent,
        job.m_remoteConnectionDataSize);

    try {
        // try to get remote node connection info from discovery man to
        // check if already discovered
        discoveryRemoteNodeInfo = m_refDiscoveryManager->GetNodeInfo(
            job.m_remoteConnectionHeader.m_nodeId);
    } catch (...) {
        IBNET_LOG_WARN("[%s] Cannot create connection to remote 0x%X, "
            "not discovered, yet", m_name,
            job.m_remoteConnectionHeader.m_nodeId);
        return;
    }

    // ensure connection is allocated
    __AllocateConnection(job.m_remoteConnectionHeader.m_nodeId);

    // sanity check
    if (m_connectionStates[job.m_remoteConnectionHeader.m_nodeId].m_state
            .load(std::memory_order_relaxed) <
            ConnectionState::e_StateCreated) {
        throw sys::IllegalStateException("Connected not created state");
    }

    // connect to remote if we aren't connected, yet
    if (!(m_connectionStates[job.m_remoteConnectionHeader.m_nodeId].
            IsConnectedToRemote())) {
        m_connections[job.m_remoteConnectionHeader.m_nodeId]->Connect(
            job.m_remoteConnectionHeader, job.m_remoteConnectionData,
            job.m_remoteConnectionDataSize);

        m_connectionStates[job.m_remoteConnectionHeader.m_nodeId]
            .SetConnectedToRemote();

        IBNET_LOG_INFO("[%s] Connected QP to remote %s", m_name,
            job.m_remoteConnectionHeader);
    }

    // check if remote confirmed that exchg data arrived
    if (!m_connectionStates[job.m_remoteConnectionHeader.m_nodeId]
            .IsRemoteConnected()) {
        // check if remote was able to connect
        // remote of the remote = current instance
        if (!ConnectionState::IsRemoteConnected(
                job.m_remoteConnectionHeader.m_connectionState)) {
            IBNET_LOG_DEBUG(
                "[%s] Remote 0x%X not connected thus far, sending exchg data",
                m_name, job.m_remoteConnectionHeader.m_nodeId);

            // seems like remote didn't get our exchange data, resend
            __ReplyConnectionExchgData(job.m_remoteConnectionHeader.m_nodeId,
                m_connectionStates[job.m_remoteConnectionHeader.m_nodeId]
                    .m_exchgFlags.load(std::memory_order_relaxed),
                discoveryRemoteNodeInfo.GetAddress().GetAddress());
        }
    }

    // finish connection of current instance and remote are fully connected
    if (m_connectionStates[job.m_remoteConnectionHeader.m_nodeId]
            .ConnectionExchgComplete() &&
            m_connectionStates[job.m_remoteConnectionHeader.m_nodeId]
                .m_available.load(std::memory_order_relaxed) ==
                ConnectionState::CONNECTION_NOT_AVAILABLE) {
        // sanity check
        uint8_t expectedState = ConnectionState::e_StateCreated;
        if (!m_connectionStates[job.m_remoteConnectionHeader.m_nodeId].m_state
                .compare_exchange_strong(expectedState,
                ConnectionState::e_StateConnected, std::memory_order_relaxed)) {
            throw sys::IllegalStateException("Illegal connection state");
        }

        m_openConnections++;

        m_connectionStates[job.m_remoteConnectionHeader.m_nodeId].m_available
            .store(ConnectionState::CONNECTION_AVAILABLE,
            std::memory_order_relaxed);

        IBNET_LOG_INFO("[%s] Connection completed with remote %s", m_name,
            job.m_remoteConnectionHeader.m_nodeId);

        _ConnectionOpened(
            *m_connections[job.m_remoteConnectionHeader.m_nodeId]);

        if (m_listener) {
            m_listener->NodeConnected(*m_connections[
                job.m_remoteConnectionHeader.m_nodeId]);
        }
    }

    // check if the current node didn't figure out that the remote
    // died and was restarted (current node acting as receiver only).
    // this results in still owning old queue pair information
    // which cannot be re-used with the new remote
    if (m_connectionStates[job.m_remoteConnectionHeader.m_nodeId]
            .IsRemoteConnected() &&
            m_connections[job.m_remoteConnectionHeader.m_nodeId]->
                GetRemoteConnectionManIdent() !=
                job.m_remoteConnectionHeader.m_conManIdent) {

        // different connection manager though same node id
        // -> application restarted, kill old connection
        IBNET_LOG_DEBUG("[%s] Detected zombie connection to node 0x%X"
            " (%X != %X), killing...", m_name,
            job.m_remoteConnectionHeader.m_nodeId,
            m_connections[job.m_remoteConnectionHeader.m_nodeId]->
                GetRemoteConnectionManIdent());

        __AddJobCloseConnection(job.m_remoteConnectionHeader.m_nodeId,
            true, false);
        __AddJobCreateConnection(job.m_remoteConnectionHeader.m_nodeId);

        return;
    }
}

void ConnectionManager::__JobDispatchCloseConnection(
        const JobCloseConnection& job)
{
    IBNET_LOG_INFO("[%s] Closing connection of 0x%X, force %d", m_name,
        job.m_nodeId, job.m_force);


    int32_t counter = m_connectionStates[job.m_nodeId].m_available.exchange(
        ConnectionState::CONNECTION_CLOSING, std::memory_order_relaxed);

    if (!job.m_force) {
        // wait until remaining threads returned the connection
        while (true) {
            int32_t tmp = m_connectionStates[job.m_nodeId].m_available.load(
                std::memory_order_relaxed);

            if (ConnectionState::CONNECTION_CLOSING - counter == tmp) {
                break;
            }

            std::this_thread::yield();
        }
    }

    // remove connection
    Connection* connection = m_connections[job.m_nodeId];
    m_connections[job.m_nodeId] = nullptr;

    // check if someone else was faster and removed it already
    if (connection == nullptr) {
        return;
    }

    connection->Close(job.m_force);
    m_connectionStates[job.m_nodeId].m_state.store(
        ConnectionState::e_StateNotAvailable, std::memory_order_relaxed);

    // re-use connection id
    m_availableConnectionIds.push_back(connection->GetConnectionId());

    delete connection;

    m_openConnections--;

    m_connectionStates[job.m_nodeId].m_available.store(
        ConnectionState::CONNECTION_NOT_AVAILABLE, std::memory_order_relaxed);

    m_refDiscoveryManager->Invalidate(job.m_nodeId, job.m_shutdown);

    _ConnectionClosed(job.m_nodeId);

    if (m_listener) {
        m_listener->NodeDisconnected(job.m_nodeId);
    }

    IBNET_LOG_DEBUG("[%s] Connection of 0x%X, force %d closed", m_name,
        job.m_nodeId, job.m_force);
}

bool ConnectionManager::__AllocateConnection(NodeId remoteNodeId)
{
    // allocate connection if necessary
    if (m_connections[remoteNodeId] == nullptr) {
        ConnectionId connectionId = m_availableConnectionIds.back();
        m_availableConnectionIds.pop_back();

        Connection* connection = _CreateConnection(connectionId);
        connection->m_refState = &m_connectionStates[remoteNodeId];

        // connection setup done, make visible
        m_connections[remoteNodeId] = connection;

        m_connectionStates[remoteNodeId].m_state.store(
            ConnectionState::e_StateCreated, std::memory_order_relaxed);

        IBNET_LOG_DEBUG("[%s] Allocated new connection to remote 0x%X", m_name,
            remoteNodeId);

        return true;
    }

    return false;
}

void ConnectionManager::__ReplyConnectionExchgData(NodeId remoteNodeId,
        uint8_t connectionState, uint32_t remoteNodeIPV4)
{
    // send QP data to remote if connection established (remote might still
    // have to do that) or if we are still lacking the data

    uint8_t sendBuffer[ExchangeManager::MAX_PAKET_SIZE];
    auto* header = (RemoteConnectionHeader*) &sendBuffer[0];
    void* data = sendBuffer + sizeof(RemoteConnectionHeader);
    size_t maxSizeData = sizeof(sendBuffer) - sizeof(RemoteConnectionHeader);
    size_t actualSizeData = maxSizeData;

    header->m_nodeId = m_ownNodeId;
    header->m_connectionState = connectionState;
    header->m_lid = _GetRefDevice()->GetLid();
    header->m_conManIdent = m_connectionCtxIdent;

    m_connections[remoteNodeId]->
        CreateConnectionExchangeData(data, maxSizeData, &actualSizeData);

    IBNET_LOG_DEBUG("[%s] Reply with connection exchg data, to remote node "
        "0x%X: node id 0x%X, lid 0x%X, con state 0x%X, con man ident 0x%X",
        m_name, remoteNodeId, header->m_nodeId, header->m_connectionState,
        header->m_lid, header->m_conManIdent);

    m_refExchangeManager->SendData(m_conDataExchgPaketType, remoteNodeIPV4,
        sendBuffer, sizeof(RemoteConnectionHeader) + actualSizeData);
}

}
}