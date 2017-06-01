#ifndef IBNET_CORE_IBRECVQUEUE_H
#define IBNET_CORE_IBRECVQUEUE_H

#include <cstdint>

#include "IbCompQueue.h"
#include "IbMemReg.h"
#include "IbQueueTracker.h"
#include "IbSharedRecvQueue.h"

namespace ibnet {
namespace core {

// forward declaration
class IbQueuePair;

class IbRecvQueue
{
public:
    friend class IbQueuePair;

    IbRecvQueue(std::shared_ptr<IbDevice>& device, IbQueuePair& parentQp,
                uint16_t queueSize,
                std::shared_ptr<IbCompQueue> sharedCompQueue = nullptr,
                std::shared_ptr<IbSharedRecvQueue> sharedRecvQueue = nullptr);

    ~IbRecvQueue();

    uint32_t GetQueueSize(void) const {
        return m_queueSize;
    }

    bool IsCompQueueShared(void) const {
        return m_compQueueIsShared;
    }

    // = set ready to receive
    // make sure to call this before ready to send
    void Open(uint16_t remoteQpLid, uint32_t remoteQpPhysicalId);

    // reserve a single recv call to avoid overrunning the queue
    // true if reserve ok and continue with recv, false if queue full
    bool Reserve(void) {
        // keep track of queue size limit
        return m_compQueue->AddOutstandingCompletion();
    }

    // posts a message receive request to the QP
    void Receive(const std::shared_ptr<IbMemReg>& memReg, uint64_t workReqId = 0);

    uint32_t PollCompletion(bool blocking = true);

    uint32_t Flush(void);

private:
    IbQueuePair& m_parentQp;
    uint16_t m_queueSize;
    bool m_compQueueIsShared;

    std::shared_ptr<IbCompQueue> m_compQueue;
    std::shared_ptr<IbSharedRecvQueue> m_sharedRecvQueue;
};

}
}

#endif //IBNET_CORE_IBRECVQUEUE_H
