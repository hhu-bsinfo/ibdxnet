#ifndef IBNET_CORE_IBSENDQUEUE_H
#define IBNET_CORE_IBSENDQUEUE_H

#include <cstdint>

#include "IbCompQueue.h"
#include "IbException.h"
#include "IbMemReg.h"

namespace ibnet {
namespace core {

// forward declaration
class IbQueuePair;

class IbSendQueue
{
public:
    friend class IbQueuePair;

    IbSendQueue(std::shared_ptr<IbDevice>& device, IbQueuePair& parentQp,
                uint16_t queueSize);

    ~IbSendQueue();

    uint16_t GetQueueSize(void) const {
        return m_queueSize;
    }

    // = set ready to send
    void Open(void);

    // reserve a single send call to avoid overrunning the queue
    // true if reserve ok and continue with send, false if queue full
    inline bool Reserve(void) {
        // keep track of queue size limit
        return m_compQueue->AddOutstandingCompletion();
    }

    // if reserve was called and you figured out you don't need the reserved slot
    // revoke it to avoid reserved slots "leaking"
    inline void RevokeReservation(void) {
        if (!m_compQueue->SubOutstandingCompletion()) {
            throw IbException("Send queue outstanding completion undderrun");
        }
    }

    // posts a message send work request to the QP
    bool Send(const std::shared_ptr<IbMemReg>& memReg, uint32_t size = (uint32_t) -1, uint64_t workReqId = 0);

    uint32_t PollCompletion(bool blocking = true);

    uint32_t Flush(void);

private:
    IbQueuePair& m_parentQp;
    uint16_t m_queueSize;

    std::unique_ptr<IbCompQueue> m_compQueue;
};

}
}

#endif //IBNET_CORE_IBSENDQUEUE_H
