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

class SendThread : public sys::ThreadLoop
{
public:
    SendThread(const std::shared_ptr<core::IbMemReg>& buffer,
        const std::shared_ptr<core::IbMemReg>& flowControlBuffer,
        std::shared_ptr<SendQueues>& bufferSendQueues,
        std::shared_ptr<core::IbConnectionManager>& connectionManager);
    ~SendThread(void);

    void PrintStatistics(void);

private:
    void _BeforeRunLoop(void) override;

    void _RunLoop(void) override;

    void _AfterRunLoop(void) override;

private:
    uint32_t __ProcessFlowControl(uint16_t nodeId,
        std::shared_ptr<core::IbConnection>& connection,
        std::shared_ptr<std::atomic<uint32_t>>& flowControlData);
    uint32_t __ProcessBuffers(uint16_t nodeId,
        std::shared_ptr<core::IbConnection>& connection,
        std::shared_ptr<ibnet::sys::Queue<std::shared_ptr<SendData>>>& queue);

private:
    const std::shared_ptr<core::IbMemReg> m_flowControlBuffer;
    const std::shared_ptr<core::IbMemReg> m_buffer;
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
