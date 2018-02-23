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
     * Register a region of allocated memory with the protection domain.
     *
     * This also pins the memory.
     *
     * @note If this call fails with "Not enough resources available", ensure
     *          your current user has the capability flag which allows
     *          memory pinning set (CAP_IPC_LOCK). This is not necessary if
     *          you are running your application as root.
     * @param addr Address of an allocated buffer to register
     * @param size Size of the allocated buffer
     * @param freeOnCleanup True to free the buffer if the protection domain is
     *          destroyed, false if the caller takes care of memory management
     * @return Pointer to a IbMemReg object registered at the protected domain
     */
     // TODO update doc
    void Register(IbMemReg* refMemReg);

    void Deregister(IbMemReg* refMemReg);

    /**
     * Get the IB protection domain object
     */
    ibv_pd* GetIBProtDom() const {
        return m_ibProtDom;
    }

    uint64_t GetTotalMemoryRegionsRegistered() const {
        return m_memoryRegionsRegistered;
    }

    /**
     * Get the total amount of memory registered (in bytes)
     */
    uint64_t GetTotalMemoryRegistered() const {
        return m_totalMemRegistered;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream &operator<<(std::ostream& os, const IbProtDom& o) {
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
