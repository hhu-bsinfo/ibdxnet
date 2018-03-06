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

#ifndef IBNET_STATS_TIMELINE_HPP
#define IBNET_STATS_TIMELINE_HPP

#include "ibnet/sys/IllegalStateException.h"

#include "Operation.hpp"
#include "Time.hpp"

namespace ibnet {
namespace stats {

class Timeline : public Operation
{
public:
    /**
     * Constructor
     *
     * @param category Name for the category (for sorting), e.g. class name
     * @param name Name of the statistic operation
     * @param sectionNames Names for the sections (this also determines the total section count)
     */
    Timeline(const std::string& category, const std::string& name, const std::vector<std::string> sectionNames) :
        Operation(category, name),
        m_times(),
        m_pos(0)
    {
        for (auto& it : sectionNames) {
            m_times.push_back(Time(category, it));
        }
    }

    void Start()
    {
        m_pos = 0;
        m_times[m_pos].Start();
    }

    void NextSection()
    {
        m_times[m_pos].Stop();
        m_pos++;

        if (m_pos >= m_times.size()) {
            throw new sys::IllegalStateException("Invalid section index: %d", m_pos);
        }

        m_times[m_pos].Start();
    }

    void Stop()
    {
        if (m_pos == 0) {
            return;
        }

        m_times[m_pos].Stop();
    }

    void WriteOstream(std::ostream& os, const std::string& indent) const override
    {
        double totalTime = 0;

        for (auto& it : m_times) {
            totalTime += it.GetTotalTime(Time::Metric::e_MetricNano);
        }

        for (size_t i = 0; i < m_times.size(); i++) {
            os << indent << '(' << i << ") " << m_times[i].GetName();
            os << ": dist " << std::setprecision(2);
            os << (m_times[i].GetTotalTime(Time::Metric::e_MetricNano) / totalTime * 100) << " %;";
            m_times[i].WriteOstream(os, "");

            if (i + 1 < m_times.size()) {
                os << '\n';
            }
        }
    }

private:
    std::vector<Time> m_times;
    size_t m_pos;
};

}
}

#endif //IBNET_STATS_TIMELINE_HPP
