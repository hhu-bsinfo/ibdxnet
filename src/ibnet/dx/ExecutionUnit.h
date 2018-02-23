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

#ifndef IBNET_DX_EXECUTIONUNIT_H
#define IBNET_DX_EXECUTIONUNIT_H

#include <string>

namespace ibnet {
namespace dx {

//
// Created by nothaas on 1/30/18.
//
class ExecutionUnit
{
public:
    const std::string& GetName() const {
        return m_name;
    }

    virtual bool Dispatch() = 0;

protected:
    explicit ExecutionUnit(const std::string& name) :
        m_name(name)
    {}

    virtual ~ExecutionUnit() = default;

private:
    const std::string m_name;
};

}
}

#endif //IBNET_DX_EXECUTIONUNIT_H
