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

class RecvThread : public sys::ThreadLoop
{
public:
    RecvThread(
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<core::IbCompQueue>& sharedFlowControlRecvCQ,
        std::shared_ptr<BufferPool>& recvBufferPool,
        std::shared_ptr<BufferPool>& recvFlowControlBufferPool,
        std::shared_ptr<MessageHandler>& msgHandler,
        std::shared_ptr<std::atomic<bool>> sharedQueueInitialFill);
    ~RecvThread(void);

    void NodeConnected(core::IbConnection& connection);

    void PrintStatistics(void);

protected:
    void _BeforeRunLoop(void) override;

    void _RunLoop(void) override;

    void _AfterRunLoop(void) override;

private:
    bool __ProcessFlowControl(void);
    bool __ProcessBuffers(void);

private:
    bool m_sharedRecvCQFilled;
    std::mutex m_nodeConnectedLock;
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCQ;
    std::shared_ptr<core::IbCompQueue> m_sharedFlowControlRecvCQ;
    std::shared_ptr<BufferPool> m_recvBufferPool;
    std::shared_ptr<BufferPool> m_recvFlowControlBufferPool;
    std::shared_ptr<MessageHandler> m_messageHandler;
    std::shared_ptr<std::atomic<bool>> m_sharedQueueInitialFill;

private:
    uint64_t m_recvBytes;
    uint64_t m_recvFlowControlBytes;
    std::vector<sys::ProfileTimer> m_timers;

};

}
}

#endif //IBNET_MSG_RECVTHREAD_H
