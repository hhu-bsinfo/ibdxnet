#ifndef IBNET_SYS_TIMER_H
#define IBNET_SYS_TIMER_H

#include <chrono>

namespace ibnet {
namespace sys {

class Timer
{
public:
    Timer(bool start = false) :
        m_running(false),
        m_start(),
        m_accuNs(0)
    {
        if (start) {
            m_running = true;
            m_start = std::chrono::high_resolution_clock::now();
        }
    };

    ~Timer(void) {};

    void Start(void)
    {
        m_running = true;
        m_start = std::chrono::high_resolution_clock::now();
        m_accuNs = 0;
    }

    void Resume(void)
    {
        if (!m_running) {
            m_start = std::chrono::high_resolution_clock::now();
            m_running = true;
        }
    }

    void Stop(void)
    {
        if (m_running) {
            m_running = false;
            std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
            m_accuNs += std::chrono::duration<uint64_t, std::nano>(stop - m_start).count();
        }
    }

    bool IsRunning(void) const
    {
        return m_running;
    }

    double GetTimeSec(void)
    {
        if (m_running) {
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0 / 1000.0 / 1000.0;
        } else {
            return m_accuNs / 1000.0 / 1000.0 / 1000.0;
        }
    }

    double GetTimeMs(void)
    {
        if (m_running) {
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0 / 1000.0;
        } else {
            return m_accuNs / 1000.0 / 1000.0;
        }
    }

    double GetTimeUs(void) {
        if (m_running) {
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0;
        } else {
            return m_accuNs / 1000.0;
        }
    }

    double GetTimeNs(void) {
        if (m_running) {
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count());
        } else {
            return m_accuNs;
        }
    }

private:
    bool m_running;
    std::chrono::high_resolution_clock::time_point m_start;
    uint64_t m_accuNs;
};

}
}

#endif // IBNET_SYS_TIMER_H
