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

#ifndef IBNET_SYS_PROFILETIMER_HPP
#define IBNET_SYS_PROFILETIMER_HPP

#include <chrono>
#include <cstdint>
#include <iostream>

#include "Operation.hpp"

namespace ibnet {
namespace stats {

/**
 * Timer class to acurately measure time for profiling code sections
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
 // TODO update doc
class Time : public Operation
{
public:
    /**
     * Constructor
     *
     * @param name Name for this timer (to identify on debug output)
     */
    explicit Time(const std::string& name) :
        Operation(name),
        m_counter(0),
        m_start(),
        m_total(std::chrono::nanoseconds(0)),
        // over a year should be sufficient
        m_best(std::chrono::hours(10000)),
        m_worst(std::chrono::seconds(0))
    {};

    /**
     * Destructor
     */
    ~Time() override = default;

    /**
     * Enter a section to be profiled/measured. Call this once, requires a call
     * to exit before calling again
     */
    inline void Start()
    {
        m_counter++;
        m_start = std::chrono::high_resolution_clock::now();
    }

    /**
     * Requires a call to enter first. Finish measuring the section to be
     * profiled
     */
    inline void Stop()
    {
        std::chrono::duration<uint64_t, std::nano> delta(
            std::chrono::high_resolution_clock::now() - m_start);

        m_total += delta;

        if (delta < m_best) {
            m_best = delta;
        }

        if (delta > m_worst) {
            m_worst = delta;
        }
    }

    /**
     * Get number of times the section was profiled (enter was called)
     */
    inline uint64_t GetCounter() const {
        return m_counter;
    }

    /**
     * Get the total execution time of the section(s) enclosed by enter and
     * exit
     *
     * @return Total time in seconds
     */
    inline double GetTotalTime() const {
        return std::chrono::duration<double>(m_total).count();
    }

    /**
     * Get the total execution time of the section(s) enclosed by enter and
     * exit
     *
     * @tparam _Unit Time unit to return
     * @return Total time
     */
    template<typename _Unit>
    inline double GetTotalTime() const {
        return std::chrono::duration<double, _Unit>(m_total).count();
    }

    /**
     * Get the average execution time of the section(s) enclosed by enter and
     * exit
     *
     * @return Average execution time in seconds
     */
    inline double GetAverageTime() const
    {
        if (m_counter == 0) {
            return 0;
        } else {
            return std::chrono::duration<double>(m_total).count() / m_counter;
        }
    }

    /**
     * Get the average execution time of the section(s) enclosed by enter and
     * exit
     *
     * @tparam _Unit Time unit to return
     * @return Total time
     */
    template<typename _Unit>
    inline double GetAverageTime() const
    {
        if (m_counter == 0) {
            return 0;
        } else {
            return std::chrono::duration<double, _Unit>(m_total).count() / m_counter;
        }
    }

    /**
     * Get the best execution time of the section(s) enclosed by enter and
     * exit
     *
     * @return Best execution time in seconds
     */
    inline double GetBestTime() const {
        return std::chrono::duration<double>(m_best).count();
    }

    /**
     * Get the best execution time of the section(s) enclosed by enter and
     * exit
     *
     * @tparam _Unit Time unit to return
     * @return Best time
     */
    template<typename _Unit>
    inline double GetBestTime() const {
        return std::chrono::duration<double, _Unit>(m_best).count();
    }

    /**
     * Get the worst execution time of the section(s) enclosed by enter and
     * exit
     *
     * @return Worst execution time in seconds
     */
    inline double GetWorstTime() const {
        return std::chrono::duration<double>(m_worst).count();
    }

    /**
     * Get the worst execution time of the section(s) enclosed by enter and
     * exit
     *
     * @tparam _Unit Time unit to return
     * @return Worst time
     */
    template<typename _Unit>
    inline double GetWorstTime() const {
        return std::chrono::duration<double, _Unit>(m_worst).count();
    }

    void WriteOstream(std::ostream& os) const override {
        os << "counter " << GetCounter();
        __FormatTime(os, "total", GetTotalTime<std::nano>());
        __FormatTime(os, "avg", GetAverageTime<std::nano>());
        __FormatTime(os, "best", GetBestTime<std::nano>());
        __FormatTime(os, "worst", GetWorstTime<std::nano>());
    }

private:
    uint64_t m_counter;

    std::chrono::high_resolution_clock::time_point m_start;

    std::chrono::duration<uint64_t, std::nano> m_total;
    std::chrono::duration<uint64_t, std::nano> m_best;
    std::chrono::duration<uint64_t, std::nano> m_worst;

private:
    static inline void __FormatTime(std::ostream& os,const std::string& name,
            double timeNs) {
        if (timeNs > 1000.0 * 1000.0 * 1000.0) {
            os << ";" << name << " " << timeNs / 1000.0 * 1000.0 * 1000.0 <<
               " sec";
        } else if (timeNs > 1000.0 * 1000.0) {
            os << ";" << name << " " << timeNs / 1000.0 * 1000.0 << " ms";
        } else if (timeNs > 1000.0) {
            os << ";" << name << " " << timeNs / 1000.0 << " us";
        } else {
            os << ";" << name << " " << timeNs << " ns";
        }
    }
};

}
}

#endif //IBNET_SYS_PROFILETIMER_HPP
