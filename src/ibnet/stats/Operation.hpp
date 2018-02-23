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

namespace ibnet {
namespace stats {

//
// Created by nothaas on 2/1/18.
//
class Operation
{
public:
    explicit Operation(const std::string& name) :
        m_name(name)
    {}

    virtual ~Operation() = default;

    const std::string& GetName() const {
        return m_name;
    }

    virtual void WriteOstream(std::ostream& os) const = 0;

    friend std::ostream &operator<<(std::ostream& os, const Operation& o) {
        os << '[' << o.m_name << "] ";
        o.WriteOstream(os);
        return os;
    }

private:
    const std::string m_name;
};

}
}

#endif //IBNET_STATS_OPERATION_HPP
