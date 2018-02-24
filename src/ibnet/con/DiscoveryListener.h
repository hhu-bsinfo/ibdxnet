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

#ifndef IBNET_CON_DISCOVERYLISTENER_H
#define IBNET_CON_DISCOVERYLISTENER_H

#include <cstdint>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace con {

/**
 * Listener to attach to the DiscoveryManager to listen to
 * new node discoveries or updates
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 05.02.2018
 */
class DiscoveryListener
{
public:
    /**
     * Called if a node got discovered
     *
     * @param nodeId Node id of the node discovered
     */
    virtual void NodeDiscovered(con::NodeId nodeId) = 0;

    /**
     * Called if a node got invalidated (e.g. not reachable anymore)
     *
     * @param nodeId Node id of the node invalidated
     */
    virtual void NodeInvalidated(con::NodeId nodeId) = 0;

protected:
    DiscoveryListener() = default;

    virtual ~DiscoveryListener() = default;
};

}
}

#endif //IBNET_CON_DISCOVERYLISTENER_H
