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

#ifndef IBNET_CON_INVALIDNODEIDEXCEPTION_H
#define IBNET_CON_INVALIDNODEIDEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace con {

/**
 * Exception thrown if the specified node id (e.g. parameter) was invalid
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class InvalidNodeIdException : public sys::Exception
{
public:
    /**
     * Constructor
     */
    InvalidNodeIdException() :
        Exception("The specified node id is invalid")
    {}

    /**
     * Constructor
     *
     * @param nodeId The node id considered not valid in this case
     * @param format Printf style format message
     * @param args Parameters for format string
     */
    template<typename... Args>
    InvalidNodeIdException(con::NodeId nodeId, const std::string& format,
            Args... args) :
        Exception("The specified node id %X is not valid, reason: " + format,
            nodeId, args...)
    {}

    /**
     * Destructor
     */
    ~InvalidNodeIdException() override = default;
};

}
}

#endif //IBNET_CON_INVALIDNODEIDEXCEPTION_H
