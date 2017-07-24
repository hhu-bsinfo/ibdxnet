#include "ConnectionCreator.h"

namespace ibnet {
namespace dx {

ConnectionCreator::ConnectionCreator(uint16_t qpMaxSendReqs,
        uint16_t qpMaxRecvReqs, uint16_t qpFlowControlMaxRecvReqs,
        std::shared_ptr<core::IbSharedRecvQueue> sharedRecvQueue,
        std::shared_ptr<core::IbCompQueue> sharedRecvCompQueue,
        std::shared_ptr<core::IbSharedRecvQueue> sharedFlowControlRecvQueue,
        std::shared_ptr<core::IbCompQueue> sharedFlowControlRecvCompQueue) :
    m_qpMaxSendReqs(qpMaxSendReqs),
    m_qpMaxRecvReqs(qpMaxRecvReqs),
    m_qpFlowControlMaxRecvReqs(qpFlowControlMaxRecvReqs),
    m_sharedRecvQueue(sharedRecvQueue),
    m_sharedRecvCompQueue(sharedRecvCompQueue),
    m_sharedFlowControlRecvQueue(sharedFlowControlRecvQueue),
    m_sharedFlowControlRecvCompQueue(sharedFlowControlRecvCompQueue)
{

}

ConnectionCreator::~ConnectionCreator(void)
{

}

std::shared_ptr<core::IbConnection> ConnectionCreator::CreateConnection(
    uint16_t connectionId,
    std::shared_ptr<core::IbDevice>& device,
    std::shared_ptr<core::IbProtDom>& protDom)
{
    auto ret = std::make_shared<core::IbConnection>(connectionId, device,
        protDom);

    ret->AddQp(m_sharedRecvQueue, m_sharedRecvCompQueue, m_qpMaxRecvReqs,
        m_qpMaxSendReqs);
    // a single work request is enough because we sum up flow control data
    ret->AddQp(m_sharedFlowControlRecvQueue, m_sharedFlowControlRecvCompQueue,
        m_qpFlowControlMaxRecvReqs, 1);

    return ret;
}

}
}