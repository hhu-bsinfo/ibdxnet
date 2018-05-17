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

#ifndef IBNET_MSGRC_RECVHANDLER_H
#define IBNET_MSGRC_RECVHANDLER_H

#include <cstdint>

#include "ibnet/core/IbMemReg.h"

#include "ibnet/con/NodeId.h"

#include "IncomingRingBuffer.h"

namespace ibnet {
namespace msgrc {

/**
 * Handle received buffers and flow control data from the receive dispatcher
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 30.01.2018
 */
class RecvHandler
{
public:
    /**
     * Called when new buffer or FC data was received
     *
     * @param ringBuffer Pointer to a ring buffer struct with information about any received data
     *        (memory managed by caller)
     * @return Number of elements of ringBuffer processed
     */
    virtual uint32_t Received(const IncomingRingBuffer::RingBuffer* ringBuffer) = 0;

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
