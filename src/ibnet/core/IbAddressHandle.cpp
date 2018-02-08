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

#include "IbAddressHandle.h"

#include "ibnet/sys/Logger.hpp"
#include "IbException.h"

namespace ibnet {
namespace core {

IbAddressHandle::IbAddressHandle(
        IbGlobalRoutingHeader& ibGrh,
        IbProtDom& ibProtDom,
        uint16_t dlid,
        uint8_t serviceLevel,
        uint8_t srcPathBits,
        uint8_t staticRate,
        uint8_t portNum) :
    m_dlid(dlid),
    m_serviceLevel(serviceLevel),
    m_srcPathBits(srcPathBits),
    m_staticRate(staticRate),
    m_isGlobal(1),
    m_portNum(portNum)
{
    IBNET_LOG_TRACE_FUNC;

    struct ibv_ah_attr ah_attr = {};

    memset(&ah_attr, 0, sizeof(ah_attr));
    ah_attr.grh.dgid = ibGrh.GetDgid();
    ah_attr.grh.flow_label = ibGrh.GetFlowLabel();
    ah_attr.grh.sgid_index = ibGrh.GetSgidIndex();
    ah_attr.grh.hop_limit = ibGrh.GetHopLimit();
    ah_attr.grh.traffic_class = ibGrh.GetTrafficClass();
    ah_attr.dlid = m_dlid;
    ah_attr.sl = m_serviceLevel;
    ah_attr.src_path_bits = m_srcPathBits;
    ah_attr.static_rate = m_staticRate;
    ah_attr.is_global = m_isGlobal;
    ah_attr.port_num = m_portNum;

    m_ibAh = ibv_create_ah(ibProtDom.GetIBProtDom(), &ah_attr);
    if(m_ibAh == nullptr) {
        IBNET_LOG_ERROR("Creating address handle failed: %s", strerror(errno));
        throw IbException("Creating address handle failed");
    }

    IBNET_LOG_INFO("Created address handle for remote node with LID 0x%X and Interface ID 0x%X in Subnet 0x%X",
        m_dlid, ibGrh.GetDgid().global.interface_id, ibGrh.GetDgid().global.subnet_prefix);
}

IbAddressHandle::IbAddressHandle(
        IbProtDom& ibProtDom,
        uint16_t dlid,
        uint8_t serviceLevel,
        uint8_t srcPathBits,
        uint8_t staticRate,
        uint8_t portNum) :
    m_dlid(dlid),
    m_serviceLevel(serviceLevel),
    m_srcPathBits(srcPathBits),
    m_staticRate(staticRate),
    m_isGlobal(0),
    m_portNum(portNum)
{
    IBNET_LOG_TRACE_FUNC;

    struct ibv_ah_attr ah_attr = {};

    memset(&ah_attr, 0, sizeof(ah_attr));
    ah_attr.dlid = m_dlid;
    ah_attr.sl = m_serviceLevel;
    ah_attr.src_path_bits = m_srcPathBits;
    ah_attr.static_rate = m_staticRate;
    ah_attr.is_global = m_isGlobal;
    ah_attr.port_num = m_portNum;

    m_ibAh = ibv_create_ah(ibProtDom.GetIBProtDom(), &ah_attr);
    if(m_ibAh == nullptr) {
        IBNET_LOG_ERROR("Creating address handle failed: %S", strerror(errno));
        throw IbException("Creating address handle failed");
    }

    IBNET_LOG_INFO("Created address handle for remote node with LID 0x%X", m_dlid);
} 

IbAddressHandle::~IbAddressHandle()
{
    ibv_destroy_ah(m_ibAh);
}

}
}