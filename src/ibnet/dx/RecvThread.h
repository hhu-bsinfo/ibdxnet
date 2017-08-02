#ifndef IBNET_DX_RECVTHREAD_H
#define IBNET_DX_RECVTHREAD_H

#include <atomic>
#include <memory>
#include <mutex>

#include "ibnet/core/IbCompQueue.h"
#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbMemReg.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/ThreadLoop.h"

#include "RecvBufferPool.h"
#include "RecvHandler.h"

namespace ibnet {
namespace dx {

/**
 * Dedicated thread for receiving data.
 *
 * The thread is polling the buffer and flow control data (shared) receive
 * queues. It adds new work requests once old ones are consumed. The received
 * data is forwarded to handlers in the jvm space.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class RecvThread : public sys::ThreadLoop
{
public:
    /**
     * Constructor

     * @param connectionManager The parent connection manager
     * @param sharedRecvCQ Shared receive buffer queue to use
     * @param sharedFlowControlRecvCQ  Shared receive flow control
     *          queue to use
     * @param recvBufferPool Buffer pool to use to fill up the
     *          queues with new work requests
     * @param recvHandler Handler which forwards the received data
     */
    RecvThread(
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<core::IbCompQueue>& sharedFlowControlRecvCQ,
        std::shared_ptr<RecvBufferPool>& recvBufferPool,
        std::shared_ptr<RecvHandler>& recvHandler);

    /**
     * Destructor
     */
    ~RecvThread(void);

    /**
     * Notify the receiver thread about a new connection created
     *
     * @param connection New connection established
     */
    void NodeConnected(core::IbConnection& connection);

    /**
     * Print statistics/performance data of the receiver thread
     */
    void PrintStatistics(void);

protected:
    void _BeforeRunLoop(void) override;

    void _RunLoop(void) override;

    void _AfterRunLoop(void) override;

private:
    bool __ProcessFlowControl(void);
    bool __ProcessBuffers(void);

private:
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCQ;
    std::shared_ptr<core::IbCompQueue> m_sharedFlowControlRecvCQ;
    std::shared_ptr<RecvBufferPool> m_recvBufferPool;
    std::shared_ptr<RecvHandler> m_recvHandler;

private:
    std::atomic<bool> m_sharedQueueInitialFill;
    uint32_t m_noDataCounter;
    uint64_t m_recvBytes;
    uint64_t m_recvFlowControlBytes;
    std::vector<sys::ProfileTimer> m_timers;

};

}
}

#endif //IBNET_DX_RECVTHREAD_H
