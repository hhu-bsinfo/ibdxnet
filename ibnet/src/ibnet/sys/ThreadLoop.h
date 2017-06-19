#ifndef IBNET_SYS_THREADLOOP_H
#define IBNET_SYS_THREADLOOP_H

#include <atomic>

#include "Thread.h"

namespace ibnet {
namespace sys {

/**
 * Typical usage of a thread looping on a field until the field turns false
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class ThreadLoop : public Thread
{
public:
    /**
     * Constructor
     *
     * @param name Name of the thread (for debugging)
     */
    ThreadLoop(const std::string& name = "") :
        Thread(name),
        m_run(true)
    {}

    /**
     * Destructor
     */
    virtual ~ThreadLoop(void)
    {}

    /**
     * Signal the thread to stop. This will cause the thread to exit
     * the loop before continuing with the next iteration.
     */
    void Stop(void)
    {
        m_run = false;
        Join();
    }

protected:
    /**
     * Execute something before the thread starts looping (e.g. init/setup)
     */
    virtual void _BeforeRunLoop(void) {};

    /**
     * Run function which is looped until exit is signaled
     */
    virtual void _RunLoop(void) = 0;

    /**
     * Execute something after the loop exited and before the thread terminates
     * (e.g. cleanup)
     */
    virtual void _AfterRunLoop(void) {};

    void exitLoop(void) {
        m_run = false;
    }

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
