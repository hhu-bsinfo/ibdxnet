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

/**
 * Interface for an execution unit for the execution engine
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 30.01.2018
 */
class ExecutionUnit
{
public:
    /**
     * Get the name of the unit
     */
    const std::string& GetName() const
    {
        return m_name;
    }

    /**
     * Dispatch work to be executed. Put your code/task whatever
     * getting executed by a worker here.
     *
     * @return True if your unit executed actual work, false if there was nothing
     *         to do/process. This controls the worker to enter into an idling state
     *         if all assigned units are idling for a certain period of time.
     */
    virtual bool Dispatch() = 0;

protected:
    /**
     * Constructor
     *
     * @param name Name of the unit
     */
    explicit ExecutionUnit(const std::string& name) :
            m_name(name)
    {
    }

    /**
     * Destructor
     */
    virtual ~ExecutionUnit() = default;

private:
    const std::string m_name;
};

}
}

#endif //IBNET_DX_EXECUTIONUNIT_H
