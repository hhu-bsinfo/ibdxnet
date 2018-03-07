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

#ifndef IBNET_CORE_IBPROTDOM_H
#define IBNET_CORE_IBPROTDOM_H

#include <memory>
#include <mutex>

#include "IbDevice.h"
#include "IbMemReg.h"

namespace ibnet {
namespace core {

/**
 * Wrapper class for a protection domain. All memory regions that are accessed
 * by the InfiniBand HCA MUST be registered with a protection domain which
 * also pins them
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbProtDom
{
public:
    /**
     * Constructor
     *
     * @param device Device to connect the protection domain to
     * @param name Name for the protection domain (for debugging)
     */
    IbProtDom(IbDevice& device, const std::string& name);

    /**
     * Destructor
     */
    ~IbProtDom();

    /**
     * Register an allocated memory region with the protection domain.
     *
     * The memory is not managed by the protection domain. The caller has to take care of
     * allocation and free'ing memory.
     *
     * @note If this call fails with "Not enough resources available", ensure
     *          your current user has the capability flag which allows
     *          memory pinning set (CAP_IPC_LOCK). This is not necessary if
     *          you are running your application as root.
     * @param refMemReg Pointer to memory region to register (caller has to manage pointer)
     */
    void Register(IbMemReg* refMemReg);

    /**
     * Deregister an already registered memory region. Ensure to call this for every
     * memory region registered before destroying/free'ing them
     *
     * @param refMemReg Memory region to deregister
     */
    void Deregister(IbMemReg* refMemReg);

    /**
     * Get the IB protection domain object
     */
    ibv_pd* GetIBProtDom() const
    {
        return m_ibProtDom;
    }

    /**
     * Get the total number of memory regions registered
     */
    uint64_t GetTotalMemoryRegionsRegistered() const
    {
        return m_memoryRegionsRegistered;
    }

    /**
     * Get the total amount of memory registered (in bytes)
     */
    uint64_t GetTotalMemoryRegistered() const
    {
        return m_totalMemRegistered;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream& operator<<(std::ostream& os, const IbProtDom& o)
    {
        return os << o.m_name << " (" << std::dec <<
                o.GetTotalMemoryRegionsRegistered() << " regions with a total of "
                << o.GetTotalMemoryRegistered() << " bytes)";
    }

private:
    const std::string m_name;
    ibv_pd* m_ibProtDom;

    uint64_t m_memoryRegionsRegistered;
    uint64_t m_totalMemRegistered;
};

}
}

#endif // IBNET_CORE_IBPROTDOM_H
