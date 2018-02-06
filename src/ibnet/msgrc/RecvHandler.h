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

#ifndef IBNET_MSGRC_RECVHANDLER_H
#define IBNET_MSGRC_RECVHANDLER_H

#include <cstdint>

#include "ibnet/core/IbMemReg.h"

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgrc {

/**
 * Handle received buffers and flow control data from the receive dispatcher
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.07.2017
 */
class RecvHandler
{
public:
    struct ReceivedPackage
    {
        static size_t Sizeof(uint32_t maxCount) {
            return sizeof(uint32_t) + maxCount * (sizeof(con::NodeId) +
                sizeof(uint8_t) + sizeof(core::IbMemReg*) + sizeof(uint32_t));
        }

        uint32_t m_count = 0;

        struct Entry
        {
            con::NodeId m_sourceNodeId;
            uint8_t m_fcData;
            core::IbMemReg* m_data;
            void* m_dataRaw;
            uint32_t m_dataLength;
        } __attribute__((__packed__)) m_entries[];
    } __attribute__((__packed__));

    /**
     * Called when new buffer or FC data was received
     *
     * @param sourceNodeId The source node id of the data
     */
    // TODO update doc
    virtual void Received(ReceivedPackage* recvPackage) = 0;

protected:
    /**
     * Constructor
     */
    RecvHandler() = default;

    /**
     * Destructor
     */
    virtual ~RecvHandler() = default;
};

}
}

#endif //IBNET_MSGRC_RECVHANDLER_H
