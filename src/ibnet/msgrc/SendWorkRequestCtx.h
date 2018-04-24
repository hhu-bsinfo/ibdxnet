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

#ifndef IBNET_SENDWORKREQUESTCTX_H
#define IBNET_SENDWORKREQUESTCTX_H

#include <cstdint>
#include <ostream>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgrc {

/**
 * Context object attached to a work request to deliver information when work request completed on ibv_poll_cq
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 24.04.2018
 */
struct SendWorkRequestCtx
{
    con::NodeId m_targetNodeId;
    uint16_t m_fcData;
    uint32_t m_sendSize;
    uint32_t m_posFront;
    uint32_t m_posBack;
    uint32_t m_posEnd;
    uint8_t m_debug;

    /**
     * Constructor
     */
    SendWorkRequestCtx() :
            m_targetNodeId(con::NODE_ID_INVALID),
            m_fcData(0xFFFF),
            m_sendSize(0xFFFFFFFF),
            m_posFront(0xFFFFFFFF),
            m_posBack(0xFFFFFFFF),
            m_posEnd(0xFFFFFFFF),
            m_debug(0xFF)
    {

    }

    /**
     * Destructor
     */
    ~SendWorkRequestCtx() = default;

    /**
     * Enable output to an out stream
     */
    friend std::ostream& operator<<(std::ostream& os, const SendWorkRequestCtx& o)
    {
        os << "m_targetNodeId " << std::hex << o.m_targetNodeId << std::dec;
        os << ", m_fcData " << o.m_fcData;
        os << ", m_sendSize " << o.m_sendSize;
        os << ", m_posFront " << o.m_posFront;
        os << ", m_posBack " << o.m_posBack;
        os << ", m_posEnd " << o.m_posEnd;
        os << ", m_debug " << static_cast<uint16_t>(o.m_debug);

        return os;
    }
};

}
}

#endif //IBNET_SENDWORKREQUESTCTX_H
