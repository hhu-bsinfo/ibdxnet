#ifndef IBNET_DX_SENDTHREAD_H
#define IBNET_DX_SENDTHREAD_H

#include <atomic>
#include <vector>

#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbMemReg.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/ThreadLoop.h"

#include "SendBuffers.h"
#include "SendHandler.h"

namespace ibnet {
namespace dx {

/**
 * Dedicated thread for sending data.
 *
 * The thread is using the SendHandler to call into the java space to get the
 * next buffer/data to send. If data is available, it fills one or, if enough
 * data is available, multiple pinned infiniband buffers and posts them on the
 * infiniband work queue. Afterwards, the same amount of work completions are
 * polled for optimal utilization. However, handling flow control data is
 * prioritized and always comes first to avoid deadlocking.
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
     * @param sendHandler Handler which allows the thread to access the jvm
     *          space to grab the next available data/work package to send
     * @param connectionManager Parent connection manager
     */
    SendThread(uint32_t recvBufferSize, std::shared_ptr<SendBuffers> buffers,
        std::shared_ptr<SendHandler>& sendHandler,
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
    uint32_t __ProcessFlowControl(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data);

    uint32_t __ProcessBuffer(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data);

private:
    const uint32_t m_recvBufferSize;
    std::shared_ptr<SendBuffers> m_buffers;
    std::shared_ptr<SendHandler> m_sendHandler;
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;

private:
    uint16_t m_prevNodeIdWritten;
    uint32_t m_prevDataWritten;

    uint64_t m_ibSendQueueBatchCount;
    uint64_t m_ibSendQueueFullUtilizationCount;

private:
    uint64_t m_sentBytes;
    uint64_t m_sentFlowControlBytes;
    std::vector<sys::ProfileTimer> m_timers;
};

}
}

#endif //IBNET_DX_SENDTHREAD_H
