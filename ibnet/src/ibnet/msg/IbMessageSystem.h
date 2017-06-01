#ifndef IBNET_CORE_IBMESSAGESYSTEM_H
#define IBNET_CORE_IBMESSAGESYSTEM_H

#include <atomic>
#include <thread>
#include <unordered_set>

#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/Queue.h"
#include "ibnet/core/IbConnection.h"
#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbDevice.h"
#include "ibnet/core/IbDiscoveryManager.h"
#include "ibnet/core/IbMemReg.h"
#include "ibnet/core/IbProtDom.h"
#include "ibnet/core/IbSharedRecvQueue.h"

#include "BufferPool.h"
#include "Config.h"
#include "EndpointOut.h"
#include "MessageHandler.h"
#include "RecvThread.h"
#include "SendQueues.h"
#include "SendThread.h"

namespace ibnet {
namespace msg {

class IbMessageSystem : public core::IbConnectionManager::Listener, public core::IbDiscoveryManager::Listener
{
public:
    IbMessageSystem(uint16_t ownNodeId, const ibnet::core::IbNodeConf& nodeConf,
        const Config& config, std::shared_ptr<MessageHandler> messageHandler,
        std::shared_ptr<core::IbConnectionManager::Listener> connectionListener,
        std::shared_ptr<core::IbDiscoveryManager::Listener> discoveryListener);
    ~IbMessageSystem(void);

    void AddNode(const core::IbNodeConf::Entry& entry);

    bool SendMessage(uint16_t destination, void* buffer, uint32_t length);

    bool SendFlowControl(uint16_t destination, uint32_t data);

    void PrintStatus(void);

    void NodeConnected(uint16_t nodeId, core::IbConnection& connection) override;

    void NodeDisconnected(uint16_t nodeId) override;

    void NodeDiscovered(uint16_t nodeId) override;

    void NodeInvalidated(uint16_t nodeId) override;

private:
    const Config m_config;

    std::shared_ptr<core::IbDevice> m_device;
    std::shared_ptr<core::IbProtDom> m_protDom;

    std::shared_ptr<core::IbSharedRecvQueue> m_sharedRecvQueue;
    std::shared_ptr<core::IbSharedRecvQueue> m_sharedFlowControlRecvQueue;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCompQueue;
    std::shared_ptr<core::IbCompQueue> m_sharedFlowControlRecvCompQueue;
    std::shared_ptr<BufferPool> m_recvBufferPool;
    std::shared_ptr<BufferPool> m_flowControlRecvBufferPool;

    std::shared_ptr<core::IbDiscoveryManager> m_discoveryManager;
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;

    std::shared_ptr<SendQueues> m_bufferSendQueues;
    std::shared_ptr<MessageHandler> m_messageHandler;

    std::shared_ptr<core::IbConnectionManager::Listener> m_connectionListener;
    std::shared_ptr<core::IbDiscoveryManager::Listener> m_discoveryListener;

    std::vector<std::unique_ptr<RecvThread>> m_recvThreads;
    std::vector<std::unique_ptr<SendThread>> m_sendThreads;

    std::shared_ptr<core::IbMemReg> __AllocAndRegisterMem(uint32_t size);
};

}
}

#endif // IBNET_CORE_IBMESSAGESYSTEM_H
