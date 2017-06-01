#ifndef IBNET_SYS_THREADLOOP_H
#define IBNET_SYS_THREADLOOP_H

#include <atomic>

#include "Thread.h"

namespace ibnet {
namespace sys {

class ThreadLoop : public Thread
{
public:
    ThreadLoop(const std::string& name = "") :
        Thread(name),
        m_run(true)
    {}

    virtual ~ThreadLoop(void)
    {}

    void Stop(void)
    {
        m_run = false;
        Join();
    }

protected:
    virtual void _BeforeRunLoop(void) {};

    virtual void _RunLoop(void) = 0;

    virtual void _AfterRunLoop(void) {};

    void _Run(void) override
    {
        _BeforeRunLoop();

        while (m_run) {
            _RunLoop();
        }

        _AfterRunLoop();
    }

private:
    std::atomic<bool> m_run;
};

}
}

#endif //IBNET_SYS_THREADLOOP_H
