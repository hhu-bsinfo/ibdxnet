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

#ifndef IBNET_STATS_DISTRIBUTION_HPP
#define IBNET_STATS_DISTRIBUTION_HPP

#include <iomanip>
#include <iostream>
#include <vector>

#include "Unit.hpp"

namespace ibnet {
namespace stats {

/**
 * Statistic operation calculating the distribution of a set of units
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 09.02.2018
 */
class Distribution : public Operation
{
public:
    /**
     * Constructor
     *
     * @param category Name for the category (for sorting), e.g. class name
     * @param name Name of the statistic operation
     * @param numUnits Number of units for this distribution
     */
    explicit Distribution(const std::string& category, const std::string& name, uint32_t numUnits) :
            Operation(category, name)
    {
        for (uint32_t i = 0; i < numUnits; i++) {
            m_units.push_back(Unit(category, name + "-DistUnit" + std::to_string(i)));
        }
    }

    /**
     * Destructor
     */
    ~Distribution() override = default;

    /**
     * Get a unit from the distribution
     *
     * @param idx Index of the unit to get
     * @return The unit specified by the index
     */
    Unit& GetUnit(size_t idx)
    {
        return m_units[idx];
    }

    Unit& GetUnit(float dist)
    {
        size_t idx = static_cast<uint32_t>(dist * m_units.size());

        if (idx >= m_units.size()) {
            idx = m_units.size() - 1;
        }

        return m_units[idx];
    }

    /**
     * Overriding virtual method
     */
    void WriteOstream(std::ostream& os, const std::string& indent) const override
    {
        uint64_t counter = 0;
        uint64_t total = 0;

        for (auto& it : m_units) {
            counter += it.GetCounter();
            total += it.GetTotalValue();
        }

        if (counter == 0) {
            counter = 1;
        }

        if (total == 0) {
            total = 1;
        }

        for (uint32_t i = 0; i < m_units.size(); i++) {
            os << indent << '(' << i << ") " << m_units[i].GetName();

            std::ios::fmtflags f(os.flags());

            os << ": dist " << std::setprecision(3) << std::fixed;
            os << ((double) m_units[i].GetCounter() / counter * 100.0) << "% ";
            os << ((double) m_units[i].GetTotalValue() / total * 100.0) << "%;";

            os.flags(f);

            m_units[i].WriteOstream(os, "");

            if (i + 1 < m_units.size()) {
                os << std::endl;
            }
        }
    }

private:
    std::vector<stats::Unit> m_units;
};

}
}

#endif //IBNET_STATS_DISTRIBUTION_HPP
