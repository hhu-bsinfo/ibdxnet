#include "IbMessageSystem.h"

#include "ibnet/core/IbQueueFullException.h"

#include "ConnectionCreator.h"
#include "MsgException.h"

namespace ibnet {
namespace msg {

IbMessageSystem::IbMessageSystem(uint16_t ownNodeId,
        const ibnet::core::IbNodeConf& nodeConf, const Config& config,
        std::shared_ptr<MessageHandler> messageHandler,
        std::shared_ptr<core::IbConnectionManager::Listener> connectionListener,
        std::shared_ptr<core::IbDiscoveryManager::Listener> discoveryListener) :
    m_config(config),
    m_device(nullptr),
    m_protDom(nullptr),
    m_sharedRecvQueue(nullptr),
    m_sharedFlowControlRecvQueue(nullptr),
    m_sharedRecvCompQueue(nullptr),
    m_sharedFlowControlRecvCompQueue(nullptr),
    m_recvBufferPool(nullptr),
    m_flowControlRecvBufferPool(nullptr),
    m_discoveryManager(nullptr),
    m_connectionManager(nullptr),
    m_bufferSendQueues(nullptr),
    m_messageHandler(messageHandler),
    m_connectionListener(connectionListener),
    m_discoveryListener(discoveryListener),
    m_recvThreads(),
    m_sendThreads()
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("Initializing message system...");

    m_device = std::make_shared<core::IbDevice>();
    m_protDom = std::make_shared<core::IbProtDom>(m_device, "message_system");

    m_sharedRecvQueue = std::make_shared<core::IbSharedRecvQueue>(m_protDom,
        m_config.m_maxRecvReqs);
    m_sharedFlowControlRecvQueue = std::make_shared<core::IbSharedRecvQueue>(
        m_protDom, m_config.m_flowControlMaxRecvReqs);

    m_sharedRecvCompQueue = std::make_shared<core::IbCompQueue>(m_device,
        m_config.m_maxRecvReqs);
    m_sharedFlowControlRecvCompQueue = std::make_shared<core::IbCompQueue>(
        m_device, m_config.m_flowControlMaxRecvReqs);

    m_recvBufferPool = std::make_shared<BufferPool>(m_config.m_inOutBufferSize,
        m_config.m_maxRecvReqs, m_protDom);
    m_flowControlRecvBufferPool = std::make_shared<BufferPool>(4,
        m_config.m_flowControlMaxRecvReqs, m_protDom);

    m_discoveryManager = std::make_shared<core::IbDiscoveryManager>(ownNodeId,
        nodeConf, 5730, m_config.m_maxNumConnections);

    m_connectionManager = std::make_shared<core::IbConnectionManager>(ownNodeId,
        5731, m_config.m_maxNumConnections, m_device, m_protDom,
        m_discoveryManager,
        std::make_unique<ConnectionCreator>(m_config.m_maxRecvReqs,
            m_config.m_maxSendReqs, m_config.m_flowControlMaxRecvReqs,
            m_config.m_flowControlMaxSendReqs, m_sharedRecvQueue,
            m_sharedRecvCompQueue, m_sharedFlowControlRecvQueue,
            m_sharedFlowControlRecvCompQueue));

    m_connectionManager->SetNodeConnectedListener(this);
    m_discoveryManager->SetNodeDiscoveryListener(this);

    IBNET_LOG_INFO("Starting {} receiver threads", m_config.m_recvThreads);

    std::shared_ptr<std::atomic<bool>> sharedQueueInitialFill =
        std::make_shared<std::atomic<bool>>(false);
    for (uint8_t i = 0; i < m_config.m_recvThreads; i++) {
        auto thread = std::make_unique<RecvThread>(
            m_connectionManager, m_sharedRecvCompQueue,
            m_sharedFlowControlRecvCompQueue, m_recvBufferPool,
            m_flowControlRecvBufferPool, m_messageHandler,
            sharedQueueInitialFill);
        thread->Start();
        m_recvThreads.push_back(std::move(thread));
    }

    m_bufferSendQueues = std::make_shared<SendQueues>(
        m_config.m_maxNumConnections, m_config.m_connectionJobPoolSize);

    IBNET_LOG_INFO("Starting {} sender threads", m_config.m_sendThreads);
    for (uint8_t i = 0; i < m_config.m_sendThreads; i++) {
        auto thread = std::make_unique<SendThread>(
            __AllocAndRegisterMem(m_config.m_inOutBufferSize),
            __AllocAndRegisterMem(sizeof(uint32_t)),
            m_bufferSendQueues, m_connectionManager);
        thread->Start();
        m_sendThreads.push_back(std::move(thread));
    }

    IBNET_LOG_INFO("Initializing message system done");
}

IbMessageSystem::~IbMessageSystem(void)
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("Shutting down message system...");

    // TODO cleanup outgoing msg queues, make sure everything's processed?
    // TODO don't allow any new messages to be put to the send queue
    // wait until everything on the send queues is sent?

    for (auto& it : m_sendThreads) {
        it->Stop();
    }
    m_sendThreads.clear();

    for (auto& it : m_recvThreads) {
        it->Stop();
    }
    m_recvThreads.clear();

    m_connectionManager.reset();
    m_discoveryManager.reset();
    m_sharedRecvCompQueue.reset();
    m_sharedRecvQueue.reset();
    m_protDom.reset();
    m_device.reset();

    IBNET_LOG_INFO("Shutting down message system done");
}

void IbMessageSystem::AddNode(const core::IbNodeConf::Entry& entry)
{
    m_discoveryManager->AddNode(entry);
}

bool IbMessageSystem::SendMessage(uint16_t destination, void* buffer,
        uint32_t length)
{
    if (length > m_config.m_inOutBufferSize) {
        throw MsgException("Buffer size (" + std::to_string(length) +
            " exceeded max buffer size (" +
            std::to_string(m_config.m_inOutBufferSize) + ")");
    }

    // establish a connection before processing the message
    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(destination);

    auto elem = std::make_shared<SendData>(destination,
        connection->GetConnectionId(), length, buffer);

    return m_bufferSendQueues->PushBack(elem);
}

bool IbMessageSystem::SendFlowControl(uint16_t destination, uint32_t data)
{
    // establish a connection before processing the message
    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(destination);

    return m_bufferSendQueues->PushBackFlowControl(
        destination, connection->GetConnectionId(), data);
}

void IbMessageSystem::PrintStatus(void)
{
    std::cout << "Buffer send queues: " << *m_bufferSendQueues << std::endl;
}

void IbMessageSystem::NodeConnected(uint16_t nodeId,
        core::IbConnection& connection)
{
    for (auto& it : m_recvThreads) {
        it->NodeConnected(connection);
    }

    // forward to external listener
    if (m_connectionListener) {
        m_connectionListener->NodeConnected(nodeId, connection);
    }
}

void IbMessageSystem::NodeDisconnected(uint16_t nodeId)
{
    // forward to external listener
    if (m_connectionListener) {
        m_connectionListener->NodeDisconnected(nodeId);
    }
}

void IbMessageSystem::NodeDiscovered(uint16_t nodeId)
{
    if (m_discoveryListener) {
        m_discoveryListener->NodeDiscovered(nodeId);
    }
}

void IbMessageSystem::NodeInvalidated(uint16_t nodeId)
{
    if (m_discoveryListener) {
        m_discoveryListener->NodeInvalidated(nodeId);
    }
}

std::shared_ptr<core::IbMemReg> IbMessageSystem::__AllocAndRegisterMem(uint32_t size)
{
    void* buffer = malloc(size);
    memset(buffer, 0, size);

    return m_protDom->Register(buffer, size, true);
}

}
}