#ifndef IBNET_JNI_DEBUGTHREAD_H
#define IBNET_JNI_DEBUGTHREAD_H

#include "ibnet/sys/ThreadLoop.h"

#include "RecvThread.h"
#include "SendThread.h"

namespace ibnet {
namespace jni {

class DebugThread : public sys::ThreadLoop
{
public:
    DebugThread(
        const std::vector<std::unique_ptr<ibnet::jni::RecvThread>>& recvThreads,
        const std::vector<std::unique_ptr<ibnet::jni::SendThread>>& sendThreads);
    ~DebugThread(void);

protected:
    void _RunLoop(void) override;

private:
    const std::vector<std::unique_ptr<ibnet::jni::RecvThread>>& m_recvThreads;
    const std::vector<std::unique_ptr<ibnet::jni::SendThread>>& m_sendThreads;
};

}
}

#endif //IBNET_JNI_DEBUGTHREAD_H
