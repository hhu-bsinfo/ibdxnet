#ifndef IBNET_MSG_TEST_IBMESSAGESYSTEMTEST_H
#define IBNET_MSG_TEST_IBMESSAGESYSTEMTEST_H

#include <memory>

#include "ibnet/msg/IbMessageSystem.h"
#include "ibnet/msg/MessageHandler.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/ProfileThroughput.hpp"
#include "ibnet/sys/ThreadLoop.h"

class IbMessageSystemTest : public ibnet::msg::MessageHandler,
    public std::enable_shared_from_this<IbMessageSystemTest>
{
public:
    IbMessageSystemTest(uint16_t ownNodeId,
        const ibnet::core::IbNodeConf& nodeConfig,
        const ibnet::msg::Config& config);
    ~IbMessageSystemTest(void);

    void SignalExit(void);

    void Execute(uint16_t remoteNodeId, uint32_t msgSizeBytes,
        uint32_t applicationThreadCount);

    void HandleMessage(uint16_t source, void* buffer, uint32_t length) override;

    void HandleFlowControlData(uint16_t source, uint32_t data) override;

private:
    class ApplicationSendThread : public ibnet::sys::ThreadLoop
    {
    public:
        ApplicationSendThread(uint16_t remoteNodeId, uint32_t msgBufferSize,
            std::shared_ptr<ibnet::msg::IbMessageSystem>& messageSystem);

        ~ApplicationSendThread(void);

        void PrintStatistics(void);

    protected:
        void _BeforeRunLoop(void) override;

        void _RunLoop(void) override;

        void _AfterRunLoop(void) override;

    private:
        uint16_t m_remoteNodeId;
        uint32_t m_msgBufferSize;
        std::shared_ptr<ibnet::msg::IbMessageSystem> m_messageSystem;

        void* m_buffer;
        ibnet::sys::ProfileTimer m_timer;
        ibnet::sys::ProfileThroughput m_throughput;
    };

private:
    uint16_t m_ownNodeId;
    const ibnet::core::IbNodeConf& m_nodeConfig;
    const ibnet::msg::Config& m_config;

    std::atomic<bool> m_exit;

    std::shared_ptr<ibnet::msg::IbMessageSystem> m_system;
    ibnet::sys::ProfileThroughput m_recvThroughput[ibnet::core::IbNodeId::MAX_NUM_NODES];
    ibnet::sys::ProfileThroughput m_recvFcThroughput[ibnet::core::IbNodeId::MAX_NUM_NODES];
    uint64_t m_recvFcData[ibnet::core::IbNodeId::MAX_NUM_NODES];

    void __PrintReceiverStats(void);
};

#endif //IBNET_MSG_TEST_IBMESSAGESYSTEMTEST_H
