#ifndef IBNET_CORE_IBREMOTEINFO_H
#define IBNET_CORE_IBREMOTEINFO_H

#include <cstdint>
#include <iostream>

#include "IbNodeId.h"

namespace ibnet {
namespace core {

class IbRemoteInfo
{
public:
    IbRemoteInfo(void) :
            m_nodeId(IbNodeId::INVALID),
            m_lid((uint16_t) -1)
    {}

    IbRemoteInfo(uint16_t nodeId, uint16_t lid,
            const std::vector<uint32_t>& physicalQpIds) :
        m_nodeId(nodeId),
        m_lid(lid),
        m_physicalQpIds(physicalQpIds)
    {}

    ~IbRemoteInfo(void)
    {}

    bool IsValid(void) const {
        return m_nodeId != IbNodeId::INVALID;
    }

    uint16_t GetNodeId(void) const {
        return m_nodeId;
    }

    uint16_t GetLid(void) const {
        return m_lid;
    }

    const std::vector<uint32_t>& GetPhysicalQpIds(void) const {
        return m_physicalQpIds;
    }

    friend std::ostream &operator<<(std::ostream& os, const IbRemoteInfo& o) {
        std::ostream& ret = os << "NodeId: 0x" << std::hex << o.m_nodeId <<
            ", Lid: 0x" << std::hex << o.m_lid;

        for (auto& it : o.m_physicalQpIds) {
            os << ", Physical QP Id: 0x" << std::hex << it;
        }

        return os;
    }

private:
    uint16_t m_nodeId;
    uint16_t m_lid;
    std::vector<uint32_t> m_physicalQpIds;
};

}
}

#endif // IBNET_CORE_IBREMOTEINFO_H
