#ifndef IBNET_CORE_IBMEMREG_H
#define IBNET_CORE_IBMEMREG_H

#include <cstddef>
#include <cstdint>
#include <iostream>

#include <infiniband/verbs.h>

#include "ibnet/sys/Assert.h"
#include "ibnet/sys/StringUtils.h"

#include "IbException.h"

namespace ibnet {
namespace core {

// forward declaration
class IbProtDom;

/**
 * A memory region that can be registered with a protection domain bound to
 * a HCA
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbMemReg
{
public:
    friend class IbProtDom;

    /**
     * Constructor
     *
     * @param addr Address of an allocated memory region
     * @param size Size of the allocated memory region
     * @param freeOnCleanup True if the allocated memory should be free'd with
     *          destruction of this object, false to leave management to the
     *          caller.
     */
    IbMemReg(void* addr, uint32_t size, bool freeOnCleanup = true) :
        m_addr(addr),
        m_size(size),
        m_freeOnCleanup(freeOnCleanup),
        m_ibMemReg(nullptr) {
        IBNET_ASSERT_PTR(addr);
    }

    /**
     * Get the local key assigned to the region when registered with a
     * protection domain
     */
    uint32_t GetLKey(void) const {
        return m_ibMemReg->lkey;
    }

    /**
     * Get the remote key assigned to the region when registered with a
     * protection domain
     */
    uint32_t GetRKey(void) const {
        return m_ibMemReg->rkey;
    }

    /**
     * Get the address/pointer of the allocated memory
     */
    void* GetAddress(void) const {
        return m_addr;
    }

    /**
     * Get the size of the allocated memory region
     */
    uint32_t GetSize(void) const {
        return m_size;
    }

    /**
     * Copy some data to the memory region
     *
     * @param data Pointer to data to copy
     * @param offset Offset inside memory region to start copying to
     * @param length Number of bytes to copy
     */
    void Memcpy(void* data, uint32_t offset, uint32_t length) {
        if (offset > m_size) {
            throw IbException("Memcpy to IbMemReg failed: offset > m_size");
        }

        memcpy((void*) ((uintptr_t) m_addr + offset), data, length);
    }

    std::string ToString(void) const {
        std::string str;
        str += sys::StringUtils::ToHexString(m_ibMemReg->lkey);
        str += ", " + sys::StringUtils::ToHexString(m_ibMemReg->rkey);
        str += ", " + sys::StringUtils::ToHexString((uintptr_t) m_addr);
        str += ", " + std::to_string(m_size);
        str += ", " + std::to_string(m_freeOnCleanup);

        return str;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream &operator<<(std::ostream& os, const IbMemReg& o) {
        return os << "0x" << std::hex << o.m_ibMemReg->lkey
                  << ", 0x" << std::hex << o.m_ibMemReg->rkey
                  << ", 0x" << std::hex << (uintptr_t) o.m_addr
                  << ", " <<  std::dec << o.m_size
                  << ", " << o.m_freeOnCleanup;
    }

private:
    void* m_addr;
    uint32_t m_size;
    bool m_freeOnCleanup;

    ibv_mr* m_ibMemReg;

private:
    /**
     * Destructor
     */
    ~IbMemReg(void) {
        if (m_freeOnCleanup) {
            free(m_addr);
        }
    }
};

}
}

#endif // IBNET_CORE_IBMEMREG_H
