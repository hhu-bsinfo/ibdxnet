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

#ifndef IBNET_STATS_TIMELINEFRAGMENTED_HPP
#define IBNET_STATS_TIMELINEFRAGMENTED_HPP

#include "ibnet/sys/IllegalStateException.h"

#include "Operation.hpp"
#include "Time.hpp"

namespace ibnet {
namespace stats {

/**
 * Statistic operation to time multiple sections to generate a full breakdown
 * Use this version of you don't have a strict linear timeline, e.g. with branches or sections you
 * don't want to time. You have to create separate timers and time the sections yourself.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 06.03.2018
 */
class TimelineFragmented : public Operation
{
public:
    /**
     * Constructor
     *
     * @param category Name for the category (for sorting), e.g. class name
     * @param name Name of the statistic operation
     * @param refTotalTime Time ref for total time used for the timeline
     * @param refTimes Time refs to use for the timeline
     */
    TimelineFragmented(const std::string& category, const std::string& name, const Time* refTotalTime,
            const std::vector<const Time*> refTimes) :
            Operation(category, name),
            m_refTotalTime(refTotalTime),
            m_refTimes(refTimes)
    {

    }

    /**
     * Overriding virtual method
     */
    void WriteOstream(std::ostream& os, const std::string& indent) const override
    {
        double totalTime = m_refTotalTime->GetTotalTime(Time::Metric::e_MetricNano);

        os << indent << "Total time: ";

        m_refTotalTime->WriteOstream(os, "");

        os << std::endl;

        for (size_t i = 0; i < m_refTimes.size(); i++) {
            os << indent << '(' << i << ") " << m_refTimes[i]->GetName();

            std::ios::fmtflags f(os.flags());

            os << ": dist " << std::setprecision(2) << std::fixed;
            os << (m_refTimes[i]->GetTotalTime(Time::Metric::e_MetricNano) / totalTime * 100) << " %;";

            os.flags(f);

            m_refTimes[i]->WriteOstream(os, "");

            if (i + 1 < m_refTimes.size()) {
                os << std::endl;
            }
        }
    }

private:
    const Time* m_refTotalTime;
    std::vector<const Time*> m_refTimes;
};

}
}

#endif //IBNET_STATS_TIMELINEFRAGMENTED_HPP
