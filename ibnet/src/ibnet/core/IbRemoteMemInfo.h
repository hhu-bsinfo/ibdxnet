#ifndef IBNET_CORE_IBREMOTEMEMINFO_H
#define IBNET_CORE_IBREMOTEMEMINFO_H

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace ibnet {
namespace core {

class IbRemoteMemInfo
{
public:
    IbRemoteMemInfo(uint32_t rkey, uint32_t size) :
        m_rkey(rkey),
        m_size(size)
    {}

    ~IbRemoteMemInfo(void) {};

    uint32_t GetRkey(void) const {
        return m_rkey;
    }

    uint32_t GetSize(void) const {
        return m_size;
    }

    friend std::ostream &operator<<(std::ostream& os, const IbRemoteMemInfo& o) {
        return os << "0x" << std::hex << o.m_rkey << ", " << o.m_size;
    }

private:
    uint32_t m_rkey;
    uint32_t m_size;
};

}
}

#endif // IBNET_CORE_IBREMOTEMEMINFO_H
