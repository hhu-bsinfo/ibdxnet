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

#include "IbProtDom.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace core {

IbProtDom::IbProtDom(IbDevice& device, const std::string& name) :
        m_name(name),
        m_ibProtDom(nullptr),
        m_memoryRegionsRegistered(0),
        m_totalMemRegistered(0)
{
    IBNET_LOG_INFO("[%s] Allocating protection domain", m_name);

    // allocate protection domain
    m_ibProtDom = ibv_alloc_pd(device.GetIBCtx());

    if (m_ibProtDom == nullptr) {
        throw IbException("Allocating protection domain %s failed", m_name);
    }

    IBNET_LOG_DEBUG("[%s] Allocated protection domain", m_name);
}

IbProtDom::~IbProtDom()
{
    IBNET_LOG_DEBUG("[%s] Destroying protection domain", m_name);

    if (m_totalMemRegistered > 0) {
        IBNET_LOG_WARN("[%s] Memory is still registered with the protection "
                "domain, total: %d", m_name, m_totalMemRegistered);
    }

    ibv_dealloc_pd(m_ibProtDom);

    IBNET_LOG_DEBUG("[%s] Destroying protection domain done", m_name);
}

void IbProtDom::Register(IbMemReg* refMemReg)
{
    IBNET_ASSERT(refMemReg != nullptr);
    IBNET_ASSERT(refMemReg->m_size != 0);

    if (refMemReg->m_addr == nullptr) {
        throw IbException("[%s] Registering memory region failed, null",
                m_name);
    }

    IBNET_LOG_TRACE("[%s] Registering memory region %p, size %d",
            m_name, refMemReg->m_addr, refMemReg->m_size);

    refMemReg->m_ibMemReg = ibv_reg_mr(m_ibProtDom, refMemReg->m_addr,
            refMemReg->m_size, IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE);

    if (refMemReg->m_ibMemReg == nullptr) {
        throw IbException("[%s] Registering memory region failed: %s",
                m_name, strerror(errno));
    }

    m_memoryRegionsRegistered++;
    m_totalMemRegistered += refMemReg->m_size;
}

void IbProtDom::Deregister(IbMemReg* refMemReg)
{
    IBNET_ASSERT(refMemReg != nullptr);
    IBNET_ASSERT(refMemReg->m_size != 0);

    IBNET_LOG_TRACE("[%s] Deregistering memory region %p, size %d",
            m_name, memReg.GetAddress(), memReg.GetSize());

    int ret = ibv_dereg_mr(refMemReg->m_ibMemReg);

    if (ret != 0) {
        throw IbException("[%s] Deregistering memory region failed: %s",
                m_name, strerror(errno));
    }

    refMemReg->m_ibMemReg = nullptr;

    m_memoryRegionsRegistered--;
    m_totalMemRegistered -= refMemReg->m_size;
}

}
}