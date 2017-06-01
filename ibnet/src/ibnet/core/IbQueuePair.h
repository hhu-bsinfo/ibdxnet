#ifndef IBNET_CORE_IBQUEUEPAIR_H
#define IBNET_CORE_IBQUEUEPAIR_H

#include <infiniband/verbs.h>

#include "IbDevice.h"
#include "IbProtDom.h"
#include "IbRemoteInfo.h"
#include "IbRecvQueue.h"
#include "IbSendQueue.h"

namespace ibnet {
namespace core {

class IbQueuePair
{
public:
    IbQueuePair(
        std::shared_ptr<IbDevice>& device,
        std::shared_ptr<IbProtDom>& protDom,
        uint16_t maxSendReqs,
        uint16_t maxRecvReqs,
        std::shared_ptr<IbCompQueue>& sharedRecvCompQueue,
        std::shared_ptr<IbSharedRecvQueue>& sharedRecvQueue);

    ~IbQueuePair(void);

    uint32_t GetPhysicalQpNum(void) const {
        return m_qpNum;
    }

    // used by other parts of the package. no need for the "user"
    ibv_qp* GetIbQp(void) const {
        return m_ibQp;
    }

    std::unique_ptr<IbSendQueue>& GetSendQueue(void) {
        return m_sendQueue;
    }

    std::unique_ptr<IbRecvQueue>& GetRecvQueue(void) {
        return m_recvQueue;
    }

private:
    std::unique_ptr<IbSendQueue> m_sendQueue;
    std::unique_ptr<IbRecvQueue> m_recvQueue;

    ibv_qp* m_ibQp;
    uint32_t m_qpNum;

    void __CreateQP(std::shared_ptr<IbProtDom>& protDom);
    void __SetInitState(void);

    void __Cleanup(void);
};

}
}

#endif // IBNET_CORE_IBQUEUEPAIR_H
