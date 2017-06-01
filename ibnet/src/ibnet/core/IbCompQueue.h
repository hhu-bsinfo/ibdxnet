#ifndef IBNET_CORE_IBCOMPQUEUE_H
#define IBNET_CORE_IBCOMPQUEUE_H

#include <atomic>
#include <memory>

#include "IbDevice.h"
#include "IbQueueTracker.h"

namespace ibnet {
namespace core {

class IbCompQueue
{
public:
    IbCompQueue(std::shared_ptr<IbDevice>& device, uint16_t size);
    ~IbCompQueue(void);

    uint32_t GetSize(void) const {
        return m_size;
    }

    ibv_cq* GetCQ(void) const {
        return m_cq;
    }

    inline bool AddOutstandingCompletion(void) {
        return m_outstandingComps.AddOutstanding();
    }

    inline bool SubOutstandingCompletion(void) {
        return m_outstandingComps.SubOutstanding();
    }

    inline uint16_t GetCurrentOutstandingCompletions(void) {
        return m_outstandingComps.GetCurrent();
    }

    uint32_t PollForCompletion(bool blocking = true, uint64_t* workReqId = nullptr, uint32_t* recvLength = nullptr);

    uint32_t Flush(void);

private:
    uint32_t m_size;
    ibv_cq* m_cq;

    bool m_firstWc;
    IbQueueTracker m_outstandingComps;
};

}
}

#endif //IBNET_CORE_IBCOMPQUEUE_H
