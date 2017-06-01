#ifndef IBNET_SYS_TIMER_H
#define IBNET_SYS_TIMER_H

#include <chrono>

namespace ibnet {
namespace sys {

/**
 * Timer class to measure execution time
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Timer
{
public:
    /**
     * Constructor
     *
     * @param start True to start the timer immediately after construction
     */
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

    /**
     * Destructor
     */
    ~Timer(void) {};

    /**
     * Start the timer. If the timer was already started, this causes a
     * reset/restart
     */
    void Start(void)
    {
        m_running = true;
        m_start = std::chrono::high_resolution_clock::now();
        m_accuNs = 0;
    }

    /**
     * Resume a timer after it was stopped
     */
    void Resume(void)
    {
        if (!m_running) {
            m_start = std::chrono::high_resolution_clock::now();
            m_running = true;
        }
    }

    /**
     * Stop the timer after it was started
     */
    void Stop(void)
    {
        if (m_running) {
            m_running = false;
            std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
            m_accuNs += std::chrono::duration<uint64_t, std::nano>(stop - m_start).count();
        }
    }

    /**
     * Check if the timer is "running"/was started
     *
     * @return True if running, false otherwise
     */
    bool IsRunning(void) const
    {
        return m_running;
    }

    /**
     * Get the measured time in seconds
     */
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

    /**
     * Get the measured time in milliseconds
     */
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

    /**
     * Get the measured time in microseconds
     */
    double GetTimeUs(void) {
        if (m_running) {
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0;
        } else {
            return m_accuNs / 1000.0;
        }
    }

    /**
     * Get the measured time in nanoseconds
     */
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
