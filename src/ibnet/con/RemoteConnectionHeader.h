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

#ifndef IBNET_CORE_REMOTEINFOHEADER_H
#define IBNET_CORE_REMOTEINFOHEADER_H

#include <cstdint>
#include <iostream>

#include "NodeId.h"

namespace ibnet {
namespace con {

/**
 * Information about a remote node (header, first part) which is required to
 * setup a connection
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
struct RemoteConnectionHeader
{
    con::NodeId m_nodeId;
    uint8_t m_exchgFlags;
    uint8_t m_exchgFlagsRemote;
    uint16_t m_lid;
    uint32_t m_conManIdent;

    /**
     * Constructor
     *
     * Sets invalid node info
     */
    RemoteConnectionHeader() :
            m_nodeId(NODE_ID_INVALID),
            m_exchgFlags(0),
            m_exchgFlagsRemote(0),
            m_lid(0xFFFF),
            m_conManIdent(0xFFFFFFFF)
    {
    }

    /**
     * Constructor
     *
     * @param nodeId Node id of the remote node
     * @param exchgFlags Connection exchg flags of the current instance
     * @param remoteExchgFlags Connection exchg flags the current instance
     *        knows about the remote
     * @param lid LID of the remote node
     * @param conManIdent Identifier of connection manager to detect
     *      rebooted nodes
     */
    RemoteConnectionHeader(con::NodeId nodeId, uint8_t exchgFlags,
            uint8_t remoteExchgFlags, uint16_t lid, uint32_t conManIdent) :
            m_nodeId(nodeId),
            m_exchgFlags(exchgFlags),
            m_exchgFlagsRemote(remoteExchgFlags),
            m_lid(lid),
            m_conManIdent(conManIdent)
    {
    }

    /**
     * Enable usage with out streams
     */
    friend std::ostream& operator<<(std::ostream& os,
            const RemoteConnectionHeader& o)
    {
        os << "NodeId: 0x" << std::hex << o.m_nodeId <<
                ", ExchgFlags: " << std::bitset<2>(o.m_exchgFlags) <<
                ", ExchgFlagsRemote: " << std::bitset<2>(o.m_exchgFlagsRemote) <<
                ", Lid: 0x" << std::hex << o.m_lid <<
                ", ConManIdent: " << std::hex << o.m_conManIdent;

        return os;
    }
} __attribute__((__packed__));

}
}

#endif // IBNET_CORE_REMOTEINFOHEADER_H
