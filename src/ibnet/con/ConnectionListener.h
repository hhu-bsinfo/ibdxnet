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

#ifndef IBNET_CON_CONNECTIONLISTENER_H
#define IBNET_CON_CONNECTIONLISTENER_H

#include "Connection.h"

namespace ibnet {
namespace con {

/**
 * Interface for a connection listener (node connected/disconnected)
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
class ConnectionListener
{
public:
    /**
     * Called when a remote node is successfully connected
     *
     * @param connection Connection established to the remote
     */
    virtual void NodeConnected(Connection& connection) = 0;

    /**
     * Called when a node disconnected and the connected got
     * cleaned up
     *
     * @param nodeId Node id of the remote that disconencted
     */
    virtual void NodeDisconnected(NodeId nodeId) = 0;

protected:
    ConnectionListener() = default;

    virtual ~ConnectionListener() = default;
};

}
}

#endif //IBNET_CON_CONNECTIONLISTENER_H
