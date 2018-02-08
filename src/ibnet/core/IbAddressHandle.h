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

#ifndef IBNET_CORE_IBADDRESSHANDLE_H
#define IBNET_CORE_IBADDRESSHANDLE_H

#include <infiniband/verbs.h>

#include "IbGlobalRoutingHeader.h"
#include "IbProtDom.h"

namespace ibnet {
namespace core {

/**
 * The address handle is used to determine the target of a UD-packet.
 *
 * @author Fabian Ruhland, fabian.ruhland@hhu.de, 01.12.2017
 */
class IbAddressHandle
{
public:
    /**
     * Constructor with Global Routing Header. The header is needed only,
     * if the remote node is in another subnet. This constructor should
     * only be used, if nodes are in different subnets.
     * 
     * @param ibGrh The routing header
     * @param ibProtDom The protection domain
     * @param dlid The router's LID
     * @param serviceLevel The service level (4 Bits)
     * @param srcPathBits The source path bits. Used when LMC is active in
     *        the remote node's port (Multiple LIDs for one port).
     * @param staticRate Limits the of packets being sent.
     * @param portNum The source port's number.
     */
    IbAddressHandle(
        IbGlobalRoutingHeader& ibGrh,
        IbProtDom& ibProtDom,
        uint16_t dlid,
        uint8_t serviceLevel,
        uint8_t srcPathBits,
        uint8_t staticRate,
        uint8_t portNum);

    /**
     * Constructor without Global Routing Header. This constructor should
     * only be used, if the remote node is in the same subnet.
     * 
     * @param ibProtDom The protection domain
     * @param dlid The router's LID
     * @param serviceLevel The service level (4 Bits)
     * @param srcPathBits The source path bits. Used when LMC is active in
     *        the remote node's port (Multiple LIDs for one port)
     * @param staticRate Limits the of packets being sent
     * @param portNum The source port's number
     */
    IbAddressHandle(
        IbProtDom& ibProtDom,
        uint16_t dlid,
        uint8_t serviceLevel,
        uint8_t srcPathBits,
        uint8_t staticRate,
        uint8_t portNum);

    ~IbAddressHandle(void);

    /**
     * Get the IB address handle "object". Used by other parts of the package
     * but no need for the "user"
     */
    ibv_ah *GetIbAh(void) {
        return m_ibAh;
    }

    /**
     * Get the dlid
     */
    uint16_t GetDlid(void) {
        return m_dlid;
    }

    /**
     * Get the service level
     */
    uint8_t GetServiceLevel(void) {
        return m_serviceLevel;
    } 

    /**
     * Get the source path bits
     */
    uint8_t GetSrcPathBits(void) {
        return m_srcPathBits;
    }

    /**
     * Get the static rate
     */
    uint8_t GetStaticRate(void) {
        return m_staticRate;
    }

    /**
     * true, if the address handle has been initialized with a global routing header;
     * false, if not
     */
    uint8_t GetIsGlobal(void) {
        return m_isGlobal;
    }

    /**
     * Get the port number
     */
    uint8_t GetPortNum(void) {
        return m_portNum;
    }

private:
    ibv_ah *m_ibAh;

    uint16_t m_dlid;
    uint8_t m_serviceLevel;
    uint8_t m_srcPathBits;
    uint8_t m_staticRate;
    uint8_t m_isGlobal;
    uint8_t m_portNum;
};

}
}

#endif