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

#ifndef IBNET_STATS_OPERATION_HPP
#define IBNET_STATS_OPERATION_HPP

#include <ostream>
#include <string>

#ifdef IBNET_DISABLE_STATISTICS
#define IBNET_STATS(...)
#else
#define IBNET_STATS(x) x
#endif

namespace ibnet {
namespace stats {

/**
 * Base class for a statistics operation
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.02.2018
 */
class Operation
{
public:
    /**
     * Constructor
     *
     * @param name Name of the operation
     */
    explicit Operation(const std::string& categoryName, const std::string& name) :
        m_categoryName(categoryName),
        m_name(name)
    {
    }

    /**
     * Destructor
     */
    virtual ~Operation() = default;

    /**
     * Get the category name of the operation
     */
    const std::string& GetCategoryName() const
    {
        return m_categoryName;
    }

    /**
     * Get the name of the operation
     */
    const std::string& GetName() const
    {
        return m_name;
    }

    /**
     * Write the current state of the operation as well as (debugging) info
     * to a ostream
     *
     * @param os Ostream object to write the output to
     */
    virtual void WriteOstream(std::ostream& os, const std::string& indent) const = 0;

    /**
     * Overloading << operator for printing to ostreams
     *
     * @param os Ostream to output to
     * @param o Operation to generate output for
     * @return Ostream object
     */
    friend std::ostream& operator<<(std::ostream& os, const Operation& o)
    {
        os << "  [" << o.m_name << "]\n";
        o.WriteOstream(os, "    ");
        return os;
    }

private:
    const std::string m_categoryName;
    const std::string m_name;
};

}
}

#endif //IBNET_STATS_OPERATION_HPP
