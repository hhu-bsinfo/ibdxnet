#ifndef IBNET_MSG_CONNECTIONCREATOR_H
#define IBNET_MSG_CONNECTIONCREATOR_H

#include "ibnet/core/IbCompQueue.h"
#include "ibnet/core/IbConnectionCreator.h"
#include "ibnet/core/IbSharedRecvQueue.h"

namespace ibnet {
namespace msg {

class ConnectionCreator : public ibnet::core::IbConnectionCreator
{
public:
    ConnectionCreator(uint16_t qpMaxRecvReqs, uint16_t qpMaxSendReqs,
        uint16_t qpFlowControlMaxRecvReqs, uint16_t qpFlowControlMaxSendReqs,
        std::shared_ptr<core::IbSharedRecvQueue> sharedRecvQueue,
        std::shared_ptr<core::IbCompQueue> sharedRecvCompQueue,
        std::shared_ptr<core::IbSharedRecvQueue> sharedFlowControlRecvQueue,
        std::shared_ptr<core::IbCompQueue> sharedFlowControlRecvCompQueue);
    ~ConnectionCreator(void);

    std::shared_ptr<core::IbConnection> CreateConnection(
        uint16_t connectionId,
        std::shared_ptr<core::IbDevice>& device,
        std::shared_ptr<core::IbProtDom>& protDom) override;

private:
    uint16_t m_qpMaxRecvReqs;
    uint16_t m_qpMaxSendReqs;
    uint16_t m_qpFlowControlMaxRecvReqs;
    uint16_t m_qpFlowControlMaxSendReqs;
    std::shared_ptr<core::IbSharedRecvQueue> m_sharedRecvQueue;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCompQueue;
    std::shared_ptr<core::IbSharedRecvQueue> m_sharedFlowControlRecvQueue;
    std::shared_ptr<core::IbCompQueue> m_sharedFlowControlRecvCompQueue;
};

}
}

#endif //IBNET_MSG_CONNECTIONCREATOR_H
