#ifndef IBNET_SYS_QUEUE_H
#define IBNET_SYS_QUEUE_H

#include <atomic>
#include <moodycamel/concurrentqueue.h>

namespace ibnet {
namespace sys {

/**
 * Implementation (actually a wrapper...) of a lock free multi consumer, multi
 * producer queue
 *
 * @tparam _T Element type to be used on the queue
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
template<typename _T>
class Queue
{
public:
    /**
     * Constructor
     *
     * @param size Capacity of the queue (pre-allocated)
     */
    Queue(uint32_t size) :
        m_size(size),
        m_queue(size)
    {

    }

    /**
     * Destructor
     */
    ~Queue(void)
    {

    }

    /**
     * Add an element to the back of the queue
     *
     * @param elem Element to add
     * @return True if successful, false if adding failed
     *          (most likely due to full queue)
     */
    bool PushBack(_T&& elem)
    {
        return m_queue.try_enqueue(elem);
    }

    /**
     * Add an element to the back of the queue
     *
     * @param elem Element to add
     * @return True if successful, false if adding failed
     *          (most likely due to full queue)
     */
    bool PushBack(const _T& elem)
    {
        return m_queue.try_enqueue(elem);
    }

    /**
     * Remove an element from the front of the queue
     * @param elem Reference to return the contents of the removed element to
     * @return True if removing from the front was sucessfull, false if failed
     *          (most likely due to empty queue)
     */
    bool PopFront(_T& elem)
    {
        return m_queue.try_dequeue(elem);
    }

    /**
     * Get the number of elements currently in the queue
     *
     * @note This value might be inaccurate due to the lock free nature
     * @return Number of elements in the queue
     */
    uint32_t GetElementCount(void) const {
        return m_queue.size_approx();
    }

    /**
     * Clear the queue (remove all elements)
     */
    void Clear(void)
    {
        _T dummy;
        uint8_t empty = 0;

        // poll queue empty, might return false even if not empty
        // ensure by polling a few more times
        while (empty < 10) {
            if (!m_queue.try_dequeue(dummy)) {
                empty++;
            } else {
                empty = 0;
            }
        }
    }

private:
    const uint32_t m_size;

    moodycamel::ConcurrentQueue<_T> m_queue;
};

}
}

#endif //IBNET_SYS_QUEUE_H
