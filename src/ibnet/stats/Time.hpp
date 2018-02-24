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

#ifndef IBNET_STATS_TIME_HPP
#define IBNET_STATS_TIME_HPP

#include <cstdint>
#include <iostream>

#include "ibnet/sys/Timer.hpp"

#include "Operation.hpp"

namespace ibnet {
namespace stats {

/**
 * Statistic operation to measure time
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.02.2018
 */
class Time : public Operation
{
public:
    /**
     * Time metrics
     */
    enum Metric
    {
        e_MetricNano = 1,
        e_MetricMicro = 2,
        e_MetricMilli = 3,
        e_MetricSec = 4
    };

public:
    /**
     * Constructor
     *
     * @param name Name of the statistic operation
     */
    explicit Time(const std::string& name) :
        Operation(name),
        m_counter(0),
        m_timer(),
        m_total(0),
        m_best(0xFFFFFFFFFFFFFFFF),
        m_worst(0)
    {
    };

    /**
     * Destructor
     */
    ~Time() override = default;

    /**
     * Start measuring time. A call to Stop() must follow this at the end of
     * the section you want to measure
     */
    inline void Start()
    {
        m_counter++;
        m_timer.Start();
    }

    /**
     * Stop measuring time. A call to Start() must preceed this call.
     */
    inline void Stop()
    {
        m_timer.Stop();

        uint64_t delta = m_timer.GetTimeNs();

        m_total += delta;

        if (delta < m_best) {
            m_best = delta;
        }

        if (delta > m_worst) {
            m_worst = delta;
        }
    }

    /**
     * Get the number of times the section was measured
     */
    inline uint64_t GetCounter() const
    {
        return m_counter;
    }

    /**
     * Get the total execution time of the section(s) enclosed by Start and
     * Stop
     *
     * @param metric Metric of time to return
     * @return Total time in the metric specified
     */
    inline double GetTotalTime(Metric metric = e_MetricSec) const
    {
        return m_total / ms_metricTable[metric];
    }

    /**
     * Get the average execution time of the section(s) enclosed by Start and
     * Stop
     *
     * @param metric Metric of time to return
     * @return Average time in the metric specified
     */
    inline double GetAverageTime(Metric metric = e_MetricSec) const
    {
        if (m_counter == 0) {
            return 0;
        } else {
            return (m_total / ms_metricTable[metric]) / m_counter;
        }
    }

    /**
     * Get the best execution time of the section(s) enclosed by Start and
     * Stop
     *
     * @param metric Metric of time to return
     * @return Best time in the metric specified
     */
    inline double GetBestTime(Metric metric = e_MetricSec) const
    {
        return m_best / ms_metricTable[metric];
    }

    /**
     * Get the worst execution time of the section(s) enclosed by Start and
     * Stop
     *
     * @param metric Metric of time to return
     * @return Worst time in the metric specified
     */
    inline double GetWorstTime(Metric metric = e_MetricSec) const
    {
        return m_worst / ms_metricTable[metric];
    }

    /**
     * Overriding virtual function
     */
    void WriteOstream(std::ostream& os) const override
    {
        os << "counter " << GetCounter();
        __FormatTime(os, "total", GetTotalTime(e_MetricNano));
        __FormatTime(os, "avg", GetAverageTime(e_MetricNano));
        __FormatTime(os, "best", GetBestTime(e_MetricNano));
        __FormatTime(os, "worst", GetWorstTime(e_MetricNano));
    }

private:
    static constexpr double ms_metricTable[e_MetricSec] = {
        1.0,
        1000.0,
        1000.0 * 1000.0,
        1000.0 * 1000.0 * 1000.0
    };

private:
    uint64_t m_counter;

    sys::Timer m_timer;

    uint64_t m_total;
    uint64_t m_best;
    uint64_t m_worst;

private:
    static inline void __FormatTime(std::ostream& os, const std::string& name,
        double timeNs)
    {
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

#endif //IBNET_STATS_TIME_HPP
