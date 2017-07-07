#ifndef IBNET_DX_DEBUGTHREAD_H
#define IBNET_DX_DEBUGTHREAD_H

#include "ibnet/sys/ThreadLoop.h"

#include "RecvThread.h"
#include "SendThread.h"

namespace ibnet {
namespace dx {

class DebugThread : public sys::ThreadLoop
{
public:
    DebugThread(
        const std::vector<std::unique_ptr<RecvThread>>& recvThreads,
        const std::vector<std::unique_ptr<SendThread>>& sendThreads);
    ~DebugThread(void);

protected:
    void _RunLoop(void) override;

private:
    const std::vector<std::unique_ptr<RecvThread>>& m_recvThreads;
    const std::vector<std::unique_ptr<SendThread>>& m_sendThreads;
};

}
}

#endif //IBNET_DX_DEBUGTHREAD_H
