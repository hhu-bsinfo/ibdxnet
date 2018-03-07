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

#ifndef IBNET_STATS_THROUGHPUT_HPP
#define IBNET_STATS_THROUGHPUT_HPP

#include "Time.hpp"
#include "Unit.hpp"

namespace ibnet {
namespace stats {

/**
 * Statistic operation calculating throughput of a Unit and Time operation
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.02.2018
 */
class Throughput : public Operation
{
public:
    /**
     * Constructor
     *
     * @param category Name for the category (for sorting), e.g. class name
     * @param name Name of the statistic operation
     */
    explicit Throughput(const std::string& category, const std::string& name) :
            Operation(category, name),
            m_refs(false),
            m_unit(new Unit(category, name, Unit::e_Base2)),
            m_time(new Time(category, name))
    {
    }

    /**
     * Constructor
     *
     * The operations provided are not free'd on destruction of this object.
     *
     * @param category Name for the category (for sorting), e.g. class name
     * @param name Name of the statistic operation
     * @param refUnit Pointer to a unit for the throughput (numerator)
     * @param refTime Pointer to a time for the throughput (denominator)
     */
    Throughput(const std::string& category, const std::string& name, Unit* refUnit, Time* refTime) :
            Operation(category, name),
            m_refs(true),
            m_unit(refUnit),
            m_time(refTime)
    {
    }

    /**
     * Destructor
     */
    ~Throughput() override
    {
        if (!m_refs) {
            delete m_unit;
            delete m_time;
        }
    }

    /**
     * Start measuring time
     */
    inline void Start()
    {
        m_time->Start();
    }

    /**
     * Add data sent/received/processed
     *
     * @param units Number of units to add
     */
    inline void Add(uint64_t units)
    {
        m_unit->Add(units);
    }

    /**
     * Stop the timer
     *
     * @param units Units to add before stopping
     */
    inline void Stop(uint64_t units = 0)
    {
        m_unit->Add(units);
        m_time->Stop();
    }

    /**
     * Get the current throughput
     *
     * @param metric Metric of the throughput (b, kb, mb etc)
     * @return Current throughput in the specified metric
     */
    inline double GetThroughput(Unit::Metric metric = Unit::e_MetricDefault,
            Time::Metric metricTime = Time::e_MetricSec) const
    {
        return m_unit->GetTotalValue(metric) / m_time->GetTotalTime(metricTime);
    }

    /**
     * Overriding virtual function
     */
    void WriteOstream(std::ostream& os, const std::string& indent) const override
    {
        uint64_t totalValue = m_unit->GetTotalValue();
        double totalTime = m_time->GetTotalTime();

        double throughput = totalValue / totalTime;

        for (uint8_t i = Unit::e_MetricKilo; i < Unit::e_MetricCount; i++) {

            if (throughput < m_unit->GetMetricFactor(static_cast<Unit::Metric>(i))) {
                std::ios::fmtflags f(os.flags());

                os << indent << "throughput " << std::setprecision(3) << std::fixed << (double) totalValue /
                        m_unit->GetMetricFactor(static_cast<Unit::Metric>(i - 1)) / totalTime << " " <<
                        ms_metricTableNames[i - 1];

                os.flags(f);

                break;
            }
        }
    }

private:
    static const char* ms_metricTableNames[];

private:
    const bool m_refs;
    Unit* m_unit;
    Time* m_time;
};

}
}

#endif //IBNET_STATS_THROUGHPUT_HPP
