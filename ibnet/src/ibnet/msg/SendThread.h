#ifndef IBNET_MSG_SENDTHREAD_H
#define IBNET_MSG_SENDTHREAD_H

#include <atomic>
#include <vector>

#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbMemReg.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/Queue.h"
#include "ibnet/sys/ThreadLoop.h"

#include "SendQueues.h"

namespace ibnet {
namespace msg {

/**
 * Dedicated thread for sending data.
 *
 * The thread is checking a job queue for new send jobs (SendData).
 * If data is available, it tries to poll the job queue to get
 * as many SendData objects as possible to fill up the send queue
 * for optimal utilization. This is done on both the buffer queue
 * as well as the flow control queue. However, handling flow control
 * data is prioritized.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class SendThread : public sys::ThreadLoop
{
public:
    /**
     * Constructor
     *
     * @param protDom Protection domain to register buffers at
     * @param outBufferSize Size of the buffer(s) for outgoing data
     * @param bufferQueueSize Size of send queue
     * @param bufferSendQueues Shared data structure containing jobs
     *          with data to send
     * @param connectionManager Parent connection manager
     */
    SendThread(std::shared_ptr<core::IbProtDom>& protDom,
        uint32_t outBufferSize, uint32_t bufferQueueSize,
        std::shared_ptr<SendQueues>& bufferSendQueues,
        std::shared_ptr<core::IbConnectionManager>& connectionManager);
    ~SendThread(void);

    /**
     * Print statistics/performance data
     */
    void PrintStatistics(void);

private:
    void _BeforeRunLoop(void) override;

    void _RunLoop(void) override;

    void _AfterRunLoop(void) override;

private:
    void __ProcessFlowControl(
        std::shared_ptr<core::IbConnection>& connection,
        std::shared_ptr<std::atomic<uint32_t>>& flowControlData);

    uint32_t __ProcessBuffers(
        std::shared_ptr<core::IbConnection>& connection,
        std::shared_ptr<ibnet::msg::SendQueue>& queue);

    std::shared_ptr<core::IbMemReg> __AllocAndRegisterMem(
        std::shared_ptr<core::IbProtDom>& protDom, uint32_t size);

private:
    std::shared_ptr<core::IbMemReg> m_flowControlBuffer;
    std::vector<std::shared_ptr<core::IbMemReg>> m_buffers;
    std::shared_ptr<SendQueues> m_bufferSendQueues;
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;

private:
    uint64_t m_sentBytes;
    uint64_t m_sentFlowControlBytes;
    std::vector<sys::ProfileTimer> m_timers;
};

}
}

#endif //IBNET_MSG_SENDTHREAD_H
