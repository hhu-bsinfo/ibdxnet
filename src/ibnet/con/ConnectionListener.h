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

class ConnectionListener
{
public:
    virtual void NodeConnected(Connection& connection) = 0;

    virtual void NodeDisconnected(NodeId nodeId) = 0;

protected:
    ConnectionListener() = default;

    virtual ~ConnectionListener() = default;
};

}
}

#endif //IBNET_CON_CONNECTIONLISTENER_H
