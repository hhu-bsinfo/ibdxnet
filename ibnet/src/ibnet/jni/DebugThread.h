#ifndef IBNET_JNI_DEBUGTHREAD_H
#define IBNET_JNI_DEBUGTHREAD_H

#include "ibnet/sys/ThreadLoop.h"
#include "ibnet/msg/IbMessageSystem.h"

namespace ibnet {
namespace jni {

class DebugThread : public sys::ThreadLoop
{
public:
    DebugThread(std::shared_ptr<ibnet::msg::IbMessageSystem> system);
    ~DebugThread(void);

protected:
    void _RunLoop(void) override;

private:
    std::shared_ptr<ibnet::msg::IbMessageSystem> m_system;
};

}
}

#endif //IBNET_JNI_DEBUGTHREAD_H
