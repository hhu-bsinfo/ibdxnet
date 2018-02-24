/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_SYS_TIMER_H
#define IBNET_SYS_TIMER_H

/**
 * The timer is implemented using either the rdtsc or rdtscp instruction or chrono of the STL.
 * If your machine does not support rdtscp or rdtsc, you can manually flip the compile flags
 */
//#define IBNET_SYS_TIMER_MODE_NORMAL
//#define IBNET_SYS_TIMER_MODE_RDTSC
//#define IBNET_SYS_TIMER_MODE_RDTSCP

// Default to RDTSCP for timer
#if !defined(IBNET_SYS_TIMER_MODE_NORMAL) || \
    !defined(IBNET_SYS_TIMER_MODE_RDTSC) || \
    !defined(IBNET_SYS_TIMER_MODE_RDTSCP)
#define IBNET_SYS_TIMER_MODE_RDTSCP
#endif

#include <chrono>

#ifdef IBNET_SYS_TIMER_MODE_RDTSC
#include <pt/pttsc.h>
#include <pt/ptutil.h>
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)

#include <pt/pttscp.h>
#include <pt/ptutil.h>

#endif

#include "ibnet/sys/Logger.hpp"

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
    explicit Timer(bool start = false) :
        m_running(false),
        m_start(),
        m_accuNs(0)
    {
#ifdef IBNET_SYS_TIMER_MODE_RDTSC
        if (ms_overhead == 0 || ms_cyclesPerSec == 0.0) {
            if (!pttsc_support()) {
                IBNET_LOG_ERROR("RDTSC instruction not supported. Recompile to "
                    "use normal timers instead");
                abort();
            }

            ms_overhead = pttsc_overhead(1000000);
            ms_cyclesPerSec = ptutil_cycles_per_sec(pttsc_start, pttsc_end,
                ms_overhead);

            IBNET_LOG_INFO("perf-timer RDTSC initialized: overhead %d cycles, "
                "cycles per sec %f", ms_overhead, ms_cyclesPerSec);
        }
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
        if (ms_overhead == 0 || ms_cyclesPerSec == 0.0) {
            if (!pttscp_support()) {
                IBNET_LOG_ERROR("RDTSCP instruction not supported. Recompile to "
                    "use either RTSCP or normal timers instead");
                abort();
            }

            ms_overhead = pttscp_overhead(1000000);
            ms_cyclesPerSec = ptutil_cycles_per_sec(pttscp_start, pttscp_end, ms_overhead);

            IBNET_LOG_INFO("perf-timer RDTSCP initialized: overhead %d cycles, "
                "cycles per sec %f", ms_overhead, ms_cyclesPerSec);
        }
#endif

        if (start) {
            Start();
        }
    };

    /**
     * Destructor
     */
    ~Timer() = default;

    /**
     * Start the timer. If the timer was already started, this causes a
     * reset/restart
     */
    inline void Start()
    {
        m_running = true;

#ifdef IBNET_SYS_TIMER_MODE_RDTSC
        m_start = pttsc_start();
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
        m_start = pttscp_start();
#else
        m_start = std::chrono::high_resolution_clock::now();
#endif

        m_accuNs = 0;
    }

    /**
     * Resume a timer after it was stopped
     */
    inline void Resume()
    {
        if (!m_running) {
#ifdef IBNET_SYS_TIMER_MODE_RDTSC
            m_start = pttsc_start();
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
            m_start = pttscp_start();
#else
            m_start = std::chrono::high_resolution_clock::now();
#endif
            m_running = true;
        }
    }

    /**
     * Stop the timer after it was started
     */
    inline void Stop()
    {
        if (m_running) {
            m_running = false;

#ifdef IBNET_SYS_TIMER_MODE_RDTSC
            uint64_t end = pttsc_end();
            m_accuNs += ptutil_cycles_to_ns(end - m_start - ms_overhead,
                ms_cyclesPerSec);
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
            uint64_t end = pttscp_end();
            m_accuNs += ptutil_cycles_to_ns(end - m_start - ms_overhead,
                ms_cyclesPerSec);
#else
            std::chrono::high_resolution_clock::time_point stop =
                std::chrono::high_resolution_clock::now();
            m_accuNs += std::chrono::duration<uint64_t, std::nano>(
                stop - m_start).count();
#endif
        }
    }

    /**
     * Check if the timer is "running"/was started
     *
     * @return True if running, false otherwise
     */
    inline bool IsRunning() const
    {
        return m_running;
    }

    /**
     * Get the measured time in seconds
     */
    inline double GetTimeSec()
    {
        if (m_running) {
#ifdef IBNET_SYS_TIMER_MODE_RDTSC
            return m_accuNs + ptutil_cycles_to_sec(pttsc_end() - m_start -
                ms_overhead, ms_cyclesPerSec);
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
            return m_accuNs + ptutil_cycles_to_sec(pttscp_end() - m_start -
                ms_overhead, ms_cyclesPerSec);
#else
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0 / 1000.0 / 1000.0;
#endif
        } else {
            return m_accuNs / 1000.0 / 1000.0 / 1000.0;
        }
    }

    /**
     * Get the measured time in milliseconds
     */
    inline double GetTimeMs()
    {
        if (m_running) {
#ifdef IBNET_SYS_TIMER_MODE_RDTSC
            return m_accuNs + ptutil_cycles_to_ms(pttsc_end() - m_start -
                                                   ms_overhead, ms_cyclesPerSec);
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
            return m_accuNs + ptutil_cycles_to_ms(pttscp_end() - m_start -
                ms_overhead, ms_cyclesPerSec);
#else
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0 / 1000.0;
#endif
        } else {
            return m_accuNs / 1000.0 / 1000.0;
        }
    }

    /**
     * Get the measured time in microseconds
     */
    inline double GetTimeUs()
    {
        if (m_running) {
#ifdef IBNET_SYS_TIMER_MODE_RDTSC
            return m_accuNs + ptutil_cycles_to_us(pttsc_end() - m_start -
                                                  ms_overhead, ms_cyclesPerSec);
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
            return m_accuNs + ptutil_cycles_to_us(pttscp_end() - m_start -
                ms_overhead, ms_cyclesPerSec);
#else
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count()) / 1000.0;
#endif
        } else {
            return m_accuNs / 1000.0;
        }
    }

    /**
     * Get the measured time in nanoseconds
     */
    inline uint64_t GetTimeNs()
    {
        if (m_running) {
#ifdef IBNET_SYS_TIMER_MODE_RDTSC
            return m_accuNs + ptutil_cycles_to_ns(pttsc_end() - m_start -
                                                  ms_overhead, ms_cyclesPerSec);
#elif defined(IBNET_SYS_TIMER_MODE_RDTSCP)
            return m_accuNs + ptutil_cycles_to_ns(pttscp_end() - m_start -
                ms_overhead, ms_cyclesPerSec);
#else
            return (m_accuNs + std::chrono::duration<uint64_t, std::nano>(
                    std::chrono::high_resolution_clock::now() -
                    m_start).count());
#endif
        } else {
            return m_accuNs;
        }
    }

private:
    bool m_running;
#ifdef IBNET_SYS_TIMER_MODE_NORMAL
    std::chrono::high_resolution_clock::time_point m_start;
#elif defined(IBNET_SYS_TIMER_MODE_RDTSC) || \
        defined(IBNET_SYS_TIMER_MODE_RDTSCP)
    uint64_t m_start;

    static uint64_t ms_overhead;
    static double ms_cyclesPerSec;
#else
#error "Unsupported timer mode specified"
#endif

    uint64_t m_accuNs;
};

}
}

#endif // IBNET_SYS_TIMER_H
