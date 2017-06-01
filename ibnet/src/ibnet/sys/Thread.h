#ifndef IBNET_SYS_THREAD_H
#define IBNET_SYS_THREAD_H

#include <chrono>
#include <thread>

#include "Exception.h"
#include "Logger.hpp"

namespace ibnet {
namespace sys {

class Thread
{
public:
    Thread(const std::string& name = "") :
        m_name(name),
        m_thread(nullptr)
    {}

    virtual ~Thread(void)
    {

    }

    const std::string& GetName(void) const {
        return m_name;
    }

    void Start(void)
    {
        m_thread = std::make_unique<std::thread>(&Thread::__Run, this);
    }

    void Join(void)
    {
        m_thread->join();
        m_thread.reset();
    }

protected:
    virtual void _Run(void) = 0;

    void _Yield(void)
    {
        std::this_thread::yield();
    }

    void _Sleep(uint32_t timeMs)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
    }

private:
    const std::string m_name;
    std::unique_ptr<std::thread> m_thread;

    void __Run(void)
    {
        try {
            IBNET_LOG_INFO("Started thread {}", m_name);
            _Run();
            IBNET_LOG_INFO("Finished thread {}", m_name);
        } catch (Exception& e) {
            e.PrintStackTrace();
            throw e;
        }
    }
};

}
}

#endif //IBNET_SYS_THREAD_H
