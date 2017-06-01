#ifndef IBNET_CORE_IBCONNECTIONCREATORSIMPLE_H
#define IBNET_CORE_IBCONNECTIONCREATORSIMPLE_H

#include "IbConnectionCreator.h"

namespace ibnet {
namespace core {

class IbConnectionCreatorSimple : public IbConnectionCreator
{
public:
    IbConnectionCreatorSimple(uint16_t qpMaxRecvReqs, uint16_t qpMaxSendReqs,
        std::shared_ptr<IbSharedRecvQueue> sharedRecvQueue,
        std::shared_ptr<IbCompQueue> sharedRecvCompQueue);

    ~IbConnectionCreatorSimple(void);

    std::shared_ptr<IbConnection> CreateConnection(
        uint16_t connectionId,
        std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom) override;

private:
    uint16_t m_qpMaxRecvReqs;
    uint16_t m_qpMaxSendReqs;
    std::shared_ptr<IbSharedRecvQueue> m_sharedRecvQueue;
    std::shared_ptr<IbCompQueue> m_sharedRecvCompQueue;
};

}
}

#endif //PROJECT_IBCONNECTIONCREATORSIMPLE_H
