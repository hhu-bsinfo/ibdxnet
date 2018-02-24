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

#ifndef IBNET_STATS_RATIO_HPP
#define IBNET_STATS_RATIO_HPP

#include <iomanip>
#include <iostream>

#include "Unit.hpp"

namespace ibnet {
namespace stats {

/**
 * Statistic operation calculating a ratio of two units
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 05.02.2018
 */
class Ratio : public Operation
{
public:
    /**
     * Constructor
     *
     * @param name Name of the statistic operation
     */
    explicit Ratio(const std::string& name) :
        Operation(name),
        m_refs(false),
        m_unit1(new Unit(name + "-Unit1")),
        m_unit2(new Unit(name + "-Unit2"))
    {
    }

    /**
     * Constructor
     *
     * The Units provided are not free'd on destruction of this object.
     *
     * @param name Name of the statistic operation
     * @param refUnit1 Pointer to a unit to use for the ratio (numerator)
     * @param refUnit2 Pointer to a unit to use for the ratio (denominator)
     */
    Ratio(const std::string& name, Unit* refUnit1, Unit* refUnit2) :
        Operation(name),
        m_refs(true),
        m_unit1(refUnit1),
        m_unit2(refUnit2)
    {
    }

    /**
     * Destructor
     */
    ~Ratio() override
    {
        if (!m_refs) {
            delete m_unit1;
            delete m_unit2;
        }
    }

    /**
     * Get the ratio of the Units' counters
     */
    inline double GetRatioCounter() const
    {
        double tmp = m_unit2->GetCounter();
        return m_unit1->GetCounter() / (tmp == 0.0 ? 1.0 : tmp);
    }

    /**
     * Get the ratio of the Units' total values
     */
    inline double GetRatioTotalValue() const
    {
        double tmp = m_unit2->GetTotalValue();
        return m_unit1->GetTotalValue() / (tmp == 0.0 ? 1.0 : tmp);
    }

    /**
     * Get the ratio of the Units' min values
     */
    inline double GetRatioMinValue() const
    {
        double tmp = m_unit2->GetMinValue();
        return m_unit1->GetMinValue() / (tmp == 0.0 ? 1.0 : tmp);
    }

    /**
     * Get the ratio of the Units' max values
     */
    inline double GetRatioMaxValue() const
    {
        double tmp = m_unit2->GetMaxValue();
        return m_unit1->GetMaxValue() / (tmp == 0.0 ? 1.0 : tmp);
    }

    /**
     * Overriding virtual method
     */
    void WriteOstream(std::ostream& os) const override
    {
        std::ios::fmtflags flags(os.flags());

        os << "counter " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioCounter() << ";" <<
            "total " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioTotalValue() << ";" <<
            "min " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioMinValue() << ";" <<
            "max " << std::fixed << std::setw(10) << std::setprecision(10)
            << std::setfill('0') << GetRatioMaxValue();

        os.flags(flags);
    }

private:
    const bool m_refs;
    Unit* m_unit1;
    Unit* m_unit2;
};

}
}

#endif //IBNET_STATS_RATIO_HPP
