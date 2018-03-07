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

#ifndef IBNET_CON_DUMMYCONNECTION_H
#define IBNET_CON_DUMMYCONNECTION_H

#include "ibnet/con/Connection.h"

namespace ibnet {
namespace con {

class DummyConnection : public Connection
{
public:
    /**
     * Constructor
     *
     * @param ownNodeId Node id of the current instance
     * @param connectionId Unique id assigned to the connection
     */
    DummyConnection(NodeId ownNodeId, ConnectionId connectionId);

    /**
     * Destructor
     */
    ~DummyConnection() override = default;

    /**
     * Overriding virtual function
     */
    void CreateConnectionExchangeData(void* connectionDataBuffer,
            size_t connectionDataMaxSize, size_t* connectionDataActualSize)
    override;

    /**
     * Overriding virtual function
     */
    void Connect(const con::RemoteConnectionHeader& remoteConnectionHeader,
            const void* remoteConnectionData, size_t remoteConnectionDataSize)
    override;

    /**
     * Overriding virtual function
     */
    void Close(bool force) override;
};

}
}

#endif // IBNET_CON_DUMMYCONNECTION_H
