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

#ifndef IBNET_MSGRC_CCOMMON_H
#define IBNET_MSGRC_CCOMMON_H

#include <cstdint>

#include "ibnet/con/NodeId.h"

namespace ibnet {
namespace msgrc {

/**
 * Structure for accessing data stored in the immediate data
 * field.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
struct ImmediateData
{
    con::NodeId m_sourceNodeId;
    uint8_t m_zeroLengthData : 1;
    uint8_t m_flowControlData : 7;
    uint8_t m_dummy;
} __attribute__((__packed__));

}
}

#endif //IBNET_MSGRC_CCOMMON_H
