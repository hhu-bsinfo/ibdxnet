#include "IbGlobalRoutingHeader.h"

namespace ibnet {
namespace core {

IbGlobalRoutingHeader::IbGlobalRoutingHeader(
        uint64_t subnetPrefix,
        uint64_t interfaceId,
        uint32_t flowLabel,
        uint8_t sgidIndex,
        uint8_t hopLimit,
        uint8_t trafficClass) :
    m_flowLabel(flowLabel),
    m_sgidIndex(sgidIndex),
    m_hopLimit(hopLimit),
    m_trafficClass(trafficClass)
{
    m_dgid.global.subnet_prefix = subnetPrefix;
    m_dgid.global.interface_id = interfaceId;
    
    m_ibGrh = new ibv_global_route();

    m_ibGrh->dgid.global.subnet_prefix = subnetPrefix;
    m_ibGrh->dgid.global.interface_id = interfaceId;
    m_ibGrh->flow_label = flowLabel;
    m_ibGrh->sgid_index = sgidIndex;
    m_ibGrh->hop_limit = hopLimit;
    m_ibGrh->traffic_class = trafficClass;
}

IbGlobalRoutingHeader::~IbGlobalRoutingHeader(void)
{
    delete m_ibGrh;
}

}
}