/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
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

#ifndef IBNET_CON_DISCONNECTEDEXCEPTION_H
#define IBNET_CON_DISCONNECTEDEXCEPTION_H

#include "ibnet/sys/Exception.h"

namespace ibnet {
namespace con {

/**
 * Exception thrown on node disconnect
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class DisconnectedException : public sys::Exception
{
public:
    /**
     * Constructor
     *
     * @param nodeId Node id of the remote
     */
    DisconnectedException() :
        Exception("Remote queue pair not available anymore")
    {}

    /**
     * Constructor
     *
     * @param nodeId Node id of the remote
     */
    explicit DisconnectedException(con::NodeId nodeId) :
        Exception("Remote queue pair of node 0x%X not available anymore",
            nodeId)
    {}

    /**
     * Destructor
     */
    ~DisconnectedException() override = default;
};

}
}

#endif // IBNET_CON_DISCONNECTEDEXCEPTION_H
