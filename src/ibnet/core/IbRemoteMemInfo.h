#ifndef IBNET_CORE_IBREMOTEMEMINFO_H
#define IBNET_CORE_IBREMOTEMEMINFO_H

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace ibnet {
namespace core {

/**
 * Information about a remote node's allocated and pinned memory for RDMA
 * operations
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbRemoteMemInfo
{
public:
    /**
     * Constructor
     *
     * @param rkey Remote key for memory location
     * @param size Size of remote memory location
     */
    IbRemoteMemInfo(uint32_t rkey, uint32_t size) :
        m_rkey(rkey),
        m_size(size)
    {}

    /**
     * Destructor
     */
    ~IbRemoteMemInfo(void) {};

    /**
     * Get the remote key of the remote memory location
     */
    uint32_t GetRkey(void) const {
        return m_rkey;
    }

    /**
     * Get the size of the remote memory location
     */
    uint32_t GetSize(void) const {
        return m_size;
    }

    /**
     * Enable usage with out streams
     */
    friend std::ostream &operator<<(std::ostream& os,
            const IbRemoteMemInfo& o) {
        return os << "0x" << std::hex << o.m_rkey << ", " << o.m_size;
    }

private:
    uint32_t m_rkey;
    uint32_t m_size;
};

}
}

#endif // IBNET_CORE_IBREMOTEMEMINFO_H
