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

//
// Created by nothaas on 2/1/18.
//
class Throughput : public Operation
{
public:
    explicit Throughput(const std::string& name) :
        Operation(name),
        m_refs(false),
        m_unit(new Unit(name)),
        m_time(new Time(name))
    {}

    Throughput(const std::string& name, Unit* refUnit, Time* refTime) :
        Operation(name),
        m_refs(true),
        m_unit(refUnit),
        m_time(refTime)
    {}

    ~Throughput() override
    {
        if (!m_refs) {
            delete m_unit;
            delete m_time;
        }
    }

    inline void Start()
    {
        m_time->Start();
    }

    inline void Add(uint64_t units) {
        m_unit->Add(units);
    }

    inline void Stop(uint64_t units = 0)
    {
        m_unit->Add(units);
        m_time->Stop();
    }

    inline double GetThroughput(
            Unit::Metric metric = Unit::e_MetricDefault) const {
        return m_unit->GetTotalValue(metric) / m_time->GetTotalTime();
    }

    template<typename _Time>
    inline double GetThroughput(
            Unit::Metric metric = Unit::e_MetricDefault) const {
        return m_unit->GetTotalValue(metric) / m_time->GetTotalTime<_Time>();
    }

    void WriteOstream(std::ostream& os) const override {
        for (uint8_t i = Unit::e_MetricKilo; i < Unit::e_MetricCount; i++) {

            if (m_unit->GetTotalValue() < m_unit->GetMetricFactor(
                    static_cast<Unit::Metric>(i))) {
                os << "throughput " << m_unit->GetTotalValue(
                   static_cast<Unit::Metric>(i - 1)) / m_time->GetTotalTime() <<
                   " " << ms_metricTableNames[i - 1];
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
