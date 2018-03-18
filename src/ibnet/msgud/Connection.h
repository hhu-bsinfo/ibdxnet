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

#ifndef IBNET_MSGUD_CONNECTION_H
#define IBNET_MSGUD_CONNECTION_H

#include <infiniband/verbs.h>

#include "ibnet/core/IbProtDom.h"
#include "ibnet/core/IbAddressHandle.h"

#include "ibnet/con/Connection.h"
#include "Common.h"

namespace ibnet {
namespace msgud {

/**
 * Implementation of a connection for messaging via UD using address handles
 *
 * @author Fabian Ruhland, fabian.ruhland@hhu.de, 30.01.2018
 */
class Connection : public con::Connection {
public:
    /**
     * Constructor
     *
     * @param ownNodeId Node id of the current instance
     * @param connectionId Unique id assigned to the connection
     * @param sendBufferSize Size of the send buffer (ring buffer) in bytes
     * @param physicalQPId Id of the local queue pair (created by ConnectionManager)
     * @param refProtDom Pointer to the IbProtDom (memory managed by caller)
     */
    Connection(uint16_t ownNodeId, uint16_t connectionId,
        uint32_t sendBufferSize, uint32_t physicalQPId,
        core::IbProtDom *refProtDom);

    /**
     * Destructor
     */
    ~Connection() override;

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

    core::IbMemReg* GetRefSendBuffer() const {
        return m_sendBuffer;
    }

    /**
    * Get the pointer to the send buffer memory region
    * (caller does not have to manage memory)
    */
    core::IbAddressHandle* GetRefAddressHandle() const {
        return m_ibAddressHandle;
    }

    /**
     * Get the id of the remote queue pair
     */
    uint32_t GetRemotePhysicalQpId() {
        return m_remotePhysicalQPId;
    }

    Counter* GetSendSequenceNumber() {
        return m_sendSequenceNumber;
    }
    Counter* GetRecvSequenceNumber() {
        return m_recvSequenceNumber;
    }

private:
    struct RemoteConnectionData
    {
        uint32_t m_physicalQPId;
    } __attribute__((__packed__));

private:
    const uint32_t m_sendBufferSize;
    core::IbProtDom* m_refProtDom;
    core::IbMemReg* m_sendBuffer;

    ibnet::core::IbAddressHandle* m_ibAddressHandle;
    uint32_t m_ownPhysicalQPId;
    uint32_t m_remotePhysicalQPId;

    Counter* m_sendSequenceNumber;
    Counter* m_recvSequenceNumber;

    RemoteConnectionData m_remoteConnectionData;
};

}
}

#endif