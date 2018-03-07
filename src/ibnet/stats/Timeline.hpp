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

/**
 * Statistic operation to time multiple sections to generate a full breakdown
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 06.03.2018
 */
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

    /**
     * Start with the first section to time starting the timeline
     */
    void Start()
    {
        m_pos = 0;
        m_times[m_pos].Start();
    }

    /**
     * Stop the previous section and continue to the next section
     */
    void NextSection()
    {
        m_times[m_pos].Stop();
        m_pos++;

        if (m_pos >= m_times.size()) {
            throw new sys::IllegalStateException("Invalid section index: %d", m_pos);
        }

        m_times[m_pos].Start();
    }

    /**
     * Stop the previous section and end the timeline here
     */
    void Stop()
    {
        if (m_pos == 0) {
            return;
        }

        m_times[m_pos].Stop();
    }

    /**
     * Overriding virtual method
     */
    void WriteOstream(std::ostream& os, const std::string& indent) const override
    {
        double totalTime = 0;

        for (auto& it : m_times) {
            totalTime += it.GetTotalTime(Time::Metric::e_MetricNano);
        }

        os << indent << "Total time: ";

        if (totalTime > 1000.0 * 1000.0 * 1000.0) {
            os << totalTime / (1000.0 * 1000.0 * 1000.0) << " sec";
        } else if (totalTime > 1000.0 * 1000.0) {
            os << totalTime / (1000.0 * 1000.0) << " ms";
        } else if (totalTime > 1000.0) {
            os << totalTime / 1000.0 << " us";
        } else {
            os << totalTime << " ns";
        }

        os << std::endl;

        for (size_t i = 0; i < m_times.size(); i++) {
            os << indent << '(' << i << ") " << m_times[i].GetName();

            std::ios::fmtflags f(os.flags());

            os << ": dist " << std::setprecision(2) << std::fixed;
            os << (m_times[i].GetTotalTime(Time::Metric::e_MetricNano) / totalTime * 100) << " %;";

            os.flags(f);

            m_times[i].WriteOstream(os, "");

            if (i + 1 < m_times.size()) {
                os << std::endl;
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
