#ifndef IBNET_CORE_IBCOMPQUEUE_H
#define IBNET_CORE_IBCOMPQUEUE_H

#include <atomic>
#include <memory>

#include "IbDevice.h"
#include "IbQueueTracker.h"

namespace ibnet {
namespace core {

/**
 * A completion queue for completed work requests. Can also be shared with
 * multiple queue pairs
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IbCompQueue
{
public:
    /**
     * Constructor
     *
     * @param device Pointer to a device to create the completion queue for
     * @param size Size of the completion queue
     */
    IbCompQueue(std::shared_ptr<IbDevice>& device, uint16_t size);

    /**
     * Destructor
     */
    ~IbCompQueue(void);

    /**
     * Get the size of the queue
     */
    uint32_t GetSize(void) const {
        return m_size;
    }

    /**
     * Get the IB completion queue object. Used by other parts of the package
     * but no need for the "user"
     */
    ibv_cq* GetCQ(void) const {
        return m_cq;
    }

    /**
     * Increase the outstanding completion counter keeping track of work
     * request completions
     *
     * @return True if adding successful, false if failed because queue size
     *          exceeded
     */
    inline bool AddOutstandingCompletion(void) {
        return m_outstandingComps.AddOutstanding();
    }

    /**
     * Decrease the outstanding completion counter keeping track of work
     * request completions
     *
     * @return True if decreasing successful, false if failed because queue is
     *          empty
     */
    inline bool SubOutstandingCompletion(void) {
        return m_outstandingComps.SubOutstanding();
    }

    /**
     * Get the number of currently outstanding work completions
     */
    inline uint16_t GetCurrentOutstandingCompletions(void) {
        return m_outstandingComps.GetCurrent();
    }

    /**
     * Poll for a work completion
     *
     * @param blocking If true, busy loop polling until a work completion is
     *          available, false for non blocking
     * @param workReqId Pointer to a variable to return the work request id
     *          of the completed work request to
     * @param recvLength Pointer to a variable to return the number of bytes
     *          transmitted with the completion of a work request
     * @return The queue pair id of the successfully completed work request or
     *          -1 if queue empty and no work request completed (non blocking)
     */
    uint32_t PollForCompletion(bool blocking = true, uint64_t* workReqId = nullptr, uint32_t* recvLength = nullptr);

    /**
 * Blocking poll all remaining work requests until they completed
 *
 * @return Number of remaining work requests completed
 */
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
