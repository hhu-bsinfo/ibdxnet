#ifndef IBNET_CORE_IBPROTDOM_H
#define IBNET_CORE_IBPROTDOM_H

#include <memory>

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
    IbProtDom(std::shared_ptr<IbDevice>& device, const std::string& name);

    /**
     * Destructor
     */
    ~IbProtDom(void);

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
    std::shared_ptr<IbMemReg> Register(void* addr, uint32_t size, bool freeOnCleanup = true);

    /**
     * Get the IB protection domain object
     */
    ibv_pd* GetIBProtDom(void) const {
        return m_ibProtDom;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream &operator<<(std::ostream& os, const IbProtDom& o) {
        os << o.m_name << ":";

        for (auto& it : o.m_registeredRegions) {
            os << "\n" << it;
        }

        return os;
    }

private:
    const std::string m_name;
    ibv_pd* m_ibProtDom;

    std::vector<std::shared_ptr<IbMemReg>> m_registeredRegions;
};

}
}

#endif // IBNET_CORE_IBPROTDOM_H
