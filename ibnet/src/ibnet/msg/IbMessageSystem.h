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
#include "MessageHandler.h"
#include "RecvThread.h"
#include "SendQueues.h"
#include "SendThread.h"

namespace ibnet {
namespace msg {

/**
 * Message system to easily send/receive "messages"/buffers.
 *
 * The message system uses a configurable number of send and receive threads
 * to provide high throughput for many send jobs optimizations for
 * multithreaded access/buffer posting.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class IbMessageSystem : public core::IbConnectionManager::Listener, public core::IbDiscoveryManager::Listener
{
public:
    /**
     * Constructor
     *
     * @param ownNodeId Node id of current node
     * @param nodeConf Node configuration
     * @param config Configuration for system
     * @param messageHandler Optional message handler for incoming data
     * @param connectionListener Optional connection listener
     * @param discoveryListener Optional discovery listener
     */
    IbMessageSystem(uint16_t ownNodeId, const ibnet::core::IbNodeConf& nodeConf,
        const Config& config, std::shared_ptr<MessageHandler> messageHandler,
        std::shared_ptr<core::IbConnectionManager::Listener> connectionListener,
        std::shared_ptr<core::IbDiscoveryManager::Listener> discoveryListener);

    /**
     * Destructor
     */
    ~IbMessageSystem(void);

    /**
     * Add a new node to the configuration.
     *
     * Use this to add nodes during runtime.
     *
     * @param entry New node config entry to add
     */
    void AddNode(const core::IbNodeConf::Entry& entry);

    /**
     * Send a message/buffer
     *
     * @param destination Node id of the destination
     * @param buffer Allocated buffer of data to send
     * @param length Number of bytes to send
     * @return True if sending buffer successful, false if queue full
     */
    bool SendMessage(uint16_t destination, void* buffer, uint32_t length);

    /**
     * Send flow control data
     *
     * @param destination Node id of the destination
     * @param data Flow control data to send
     * @return True if sending successful, false otherwise
     */
    bool SendFlowControl(uint16_t destination, uint32_t data);

    /**
     * Print the current status of the message system
     */
    void PrintStatus(void);

    /**
     * Callback override
     */
    void NodeConnected(uint16_t nodeId, core::IbConnection& connection) override;

    /**
     * Callback override
     */
    void NodeDisconnected(uint16_t nodeId) override;

    /**
     * Callback override
     */
    void NodeDiscovered(uint16_t nodeId) override;

    /**
     * Callback override
     */
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
