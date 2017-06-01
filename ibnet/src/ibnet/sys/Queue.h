#ifndef IBNET_SYS_QUEUE_H
#define IBNET_SYS_QUEUE_H

#include <atomic>
#include <moodycamel/concurrentqueue.h>

namespace ibnet {
namespace sys {

template<typename _T>
class Queue
{
public:
    Queue(uint32_t size) :
        m_size(size),
        m_queue(size)
    {

    }

    ~Queue(void)
    {

    }

    bool PushBack(_T&& elem)
    {
        return m_queue.try_enqueue(elem);
    }

    bool PushBack(const _T& elem)
    {
        return m_queue.try_enqueue(elem);
    }

    bool PopFront(_T& elem)
    {
        return m_queue.try_dequeue(elem);
    }

    uint32_t GetElementCount(void) const {
        return m_queue.size_approx();
    }

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
