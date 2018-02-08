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

#ifndef IBNET_CORE_IBGLOBALROUTINGHEADER_H
#define IBNET_CORE_IBGLOBALROUTINGHEADER_H

#include <infiniband/verbs.h>

namespace ibnet {
namespace core {

/**
 * The global routing header is part of an address handle.
 * It needs to be used when the target node is in another subnet.
 * 
 * @see IbAddressHandle
 * @author Fabian Ruhland, fabian.ruhland@hhu.de, 01.12.2017
 */
class IbGlobalRoutingHeader
{
public:
    /**
     * Constructor
     * 
     * @param subnetPrefix Identifies a set of endports in the tartget subnet,
     *        that are managed by the subnet manager
     * @param interfaceId EUI-64 value, that identifies the remote interface
     * @param flowLabel 20-bit value, that tells switches and routers,
     *        packets with the same flowLabel should not be reordered
     * @param sgidIndex Index in the GID-table to identify the packet's originator.
     * @param hopLimit Limits the number of hops, that a packet may pass.
     *                 Used to prevent loops
     * @param trafficClass Used to set a packets priority
     */
    IbGlobalRoutingHeader(
        uint64_t subnetPrefix,
        uint64_t interfaceId,
        uint32_t flowLabel,
        uint8_t sgidIndex,
        uint8_t hopLimit,
        uint8_t trafficClass);
    
    ~IbGlobalRoutingHeader(void);

    /**
     * Get the IB routing header "object". Used by other parts of the package
     * but no need for the "user"
     */
    ibv_global_route *GetIbGrh(void) {
        return m_ibGrh;
    }

    /**
     * Get the dgid
     */
    union ibv_gid GetDgid(void) {
        return m_dgid;
    }

    /**
     * Get the flow label
     */
    uint32_t GetFlowLabel(void) {
        return m_flowLabel;
    }

    /**
     * Get the sgid index
     */
    uint8_t GetSgidIndex(void) {
        return m_sgidIndex;
    }

    /**
     * Get the hop limit
     */
    uint8_t GetHopLimit(void) {
        return m_hopLimit;
    }

    /**
     * Get the traffic class
     */
    uint8_t GetTrafficClass(void) {
        return m_trafficClass;
    }


private:
    ibv_global_route *m_ibGrh;

    ibv_gid m_dgid;
    uint32_t m_flowLabel;
    uint8_t m_sgidIndex;
    uint8_t m_hopLimit;
    uint8_t m_trafficClass;
};

}
}

#endif