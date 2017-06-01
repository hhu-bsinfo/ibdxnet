#include "IbConnectionCreatorSimple.h"

namespace ibnet {
namespace core {

IbConnectionCreatorSimple::IbConnectionCreatorSimple(uint16_t qpMaxRecvReqs,
        uint16_t qpMaxSendReqs,
        std::shared_ptr<IbSharedRecvQueue> sharedRecvQueue,
        std::shared_ptr<IbCompQueue> sharedRecvCompQueue) :
    m_qpMaxRecvReqs(qpMaxRecvReqs),
    m_qpMaxSendReqs(qpMaxSendReqs),
    m_sharedRecvQueue(sharedRecvQueue),
    m_sharedRecvCompQueue(sharedRecvCompQueue)
{

}

IbConnectionCreatorSimple::~IbConnectionCreatorSimple(void)
{

}

std::shared_ptr<IbConnection> IbConnectionCreatorSimple::CreateConnection(
        uint16_t connectionId, std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom)
{
    auto ret = std::make_shared<IbConnection>(connectionId, device, protDom);
    ret->AddQp(m_sharedRecvQueue, m_sharedRecvCompQueue, m_qpMaxRecvReqs,
        m_qpMaxSendReqs);

    return ret;
}

}
}