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

#ifndef IBNET_CON_CONNECTION_H
#define IBNET_CON_CONNECTION_H

#include "ConnectionState.h"
#include "RemoteConnectionHeader.h"

namespace ibnet {
namespace con {

typedef uint16_t ConnectionId;

// forward declration for friendship
class ConnectionManager;

/**
 * Interface of a logical connection with a remote node. A connection can
 * consist of one or multiple queue pairs
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Connection
{
public:
    friend class ConnectionManager;

    /**
     * Constructor
     *
     * @param ownNodeId Node id of the current instance
     * @param connectionId Unique id assigned to the connection
     */
    Connection(NodeId ownNodeId, ConnectionId connectionId) :
        m_ownNodeId(ownNodeId),
        m_connectionId(connectionId),
        m_refState(nullptr),
        m_remoteConnectionHeader()
    {
    }

    /**
     * Destructor
     */
    virtual ~Connection() = default;

    /**
     * Get the node id of the source node (this instance)
     */
    NodeId GetSourceNodeId() const
    {
        return m_ownNodeId;
    }

    /**
     * Get the connection id
     */
    ConnectionId GetConnectionId() const
    {
        return m_connectionId;
    }

    /**
     * Get the node id of the remote/target node
     */
    NodeId GetRemoteNodeId() const
    {
        return m_remoteConnectionHeader.m_nodeId;
    }

    /**
     * Get the connection manager identifier of the remote
     */
    uint32_t GetRemoteConnectionManIdent() const
    {
        return m_remoteConnectionHeader.m_conManIdent;
    }

    /**
     * Check if the connection is up
     */
    bool IsConnected() const
    {
        return m_refState->IsConnected();
    }

    /**
     * Create connection exchange data to be sent on connection creation to the
     * remote node. This must include all necessary data like physical qp ids
     * rkeys etc. necessary to create or completion a connection
     *
     * @param connectionDataBuffer Buffer to write connection data to
     * @param connectionDataMaxSize Max size of data to write to the provided buffer
     * @param connectionDataActualSize Pointer to a variable to return the size of
     *        the data written to the buffer
     */
    virtual void CreateConnectionExchangeData(void* connectionDataBuffer,
        size_t connectionDataMaxSize, size_t* connectionDataActualSize) = 0;

    /**
     * Connect to a remote node
     *
     * This needs to be called before any operation on queue pairs is possible.
     * Override this and implement any queue pair setup necessary to open
     * an IB connection to the remote
     *
     * @param remoteConnectionHeader Connection header of the remote trying to
     *        establish a connection with the current instance
     * @param remoteConnectionData Pointer to a buffer with connection exchange
     *        data from the remote
     * @param remoteConnectionDataSize Size of the connection exchange data
     */
    virtual void Connect(const con::RemoteConnectionHeader& remoteConnectionHeader,
        const void* remoteConnectionData, size_t remoteConnectionDataSize)
    {
        m_remoteConnectionHeader = remoteConnectionHeader;
    }

    /**
     * Close the established connection
     *
     * Override this and implement cleanup tasks to completely close the
     * IB connection to the remote.
     *
     * @param force Force close connection and don't wait until queues are
     *        empty etc
     */
    virtual void Close(bool force) = 0;

    /**
     * Enable output to an out stream
     */
    friend std::ostream& operator<<(std::ostream& os, const Connection& o)
    {
        return os <<
            "Connection: " << std::hex << o.m_ownNodeId << " -> " <<
            std::hex << o.m_remoteConnectionHeader.m_nodeId <<
            ", connectionId " << std::dec << o.m_connectionId <<
            ", m_connectionState: " << *o.m_refState <<
            std::dec << ", m_remoteConnectionHeader: " <<
            o.m_remoteConnectionHeader;
    }

protected:
    const NodeId m_ownNodeId;
    const ConnectionId m_connectionId;
    ConnectionState* m_refState;
    RemoteConnectionHeader m_remoteConnectionHeader;
};

}
}

#endif //IBNET_CON_CONNECTION_H
