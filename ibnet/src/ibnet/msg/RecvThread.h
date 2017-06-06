#ifndef IBNET_MSG_RECVTHREAD_H
#define IBNET_MSG_RECVTHREAD_H

#include <atomic>
#include <memory>
#include <mutex>

#include "ibnet/core/IbCompQueue.h"
#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbMemReg.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/ThreadLoop.h"

#include "BufferPool.h"
#include "MessageHandler.h"

namespace ibnet {
namespace msg {

/**
 * Dedicated thread for receiving data.
 *
 * The thread is polling the buffer and flow control data receive
 * queues. Furthermore, it tries to keep the queues busy by
 * polling for multiple work requests (if available) and also
 * adding new work requests once old ones are consumed
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class RecvThread : public sys::ThreadLoop
{
public:
    /**
     * Constructor
     *
     * @param primaryRecvThread True if primary recv thread (handles some
     *          queue management on NodeConnect), false for all other threads
     * @param connectionManager The parent connection manager
     * @param sharedRecvCQ Shared receive buffer queue to use
     * @param sharedFlowControlRecvCQ  Shared receive flow control
     *          queue to use
     * @param recvBufferPool Buffer pool to use to fill up the
     *          queues with new work requests
     * @param recvFlowControlBufferPool Buffer pool for flow control
     *          data requests to fill the queues
     * @param msgHandler Message handler to dispatch incoming data to
     */
    RecvThread(
        bool primaryRecvThread,
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<core::IbCompQueue>& sharedFlowControlRecvCQ,
        std::shared_ptr<BufferPool>& recvBufferPool,
        std::shared_ptr<BufferPool>& recvFlowControlBufferPool,
        std::shared_ptr<MessageHandler>& msgHandler);
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
    bool m_primaryRecvThread;
    std::mutex m_nodeConnectedLock;
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCQ;
    std::shared_ptr<core::IbCompQueue> m_sharedFlowControlRecvCQ;
    std::shared_ptr<BufferPool> m_recvBufferPool;
    std::shared_ptr<BufferPool> m_recvFlowControlBufferPool;
    std::shared_ptr<MessageHandler> m_messageHandler;

private:
    uint64_t m_recvBytes;
    uint64_t m_recvFlowControlBytes;
    std::vector<sys::ProfileTimer> m_timers;

};

}
}

#endif //IBNET_MSG_RECVTHREAD_H
