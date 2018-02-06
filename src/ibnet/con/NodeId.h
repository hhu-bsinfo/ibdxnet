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

#ifndef IBNET_CON_IBNODEID_H
#define IBNET_CON_IBNODEID_H

#include <cstdint>

namespace ibnet {
namespace con {

/**
 * Abstract node id to identify nodes in an InfiniBand network
 * (not part of ibverbs or InfiniBand hardware)
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
typedef uint16_t NodeId;

static const uint16_t NODE_ID_INVALID = 0xFFFF;
static const uint16_t NODE_ID_MAX_NUM_NODES = 0xFFFF;

}
}

#endif // IBNET_CON_IBNODEID_H
