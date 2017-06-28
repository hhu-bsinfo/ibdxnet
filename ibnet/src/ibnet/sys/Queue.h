#ifndef IBNET_SYS_QUEUE_H
#define IBNET_SYS_QUEUE_H

// XXX moodycamel's queue is broken. When using on high load, it seems like
// the order of elements is not kept or elements are overwritten
// Use a simple and stupid mutex based variant for now and implement own
// multi producer/multi consumer lock free queue later. The performance of
// the mutex based queue isn't too bad but can be improved (up to 800/1000mb/sec
// throughput at least)
//#include <atomic>
//#include <moodycamel/concurrentqueue.h>
//
//namespace ibnet {
//namespace sys {
//
///**
// * Implementation (actually a wrapper...) of a lock free multi consumer, multi
// * producer queue
// *
// * @tparam _T Element type to be used on the queue
// * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
// */
//template<typename _T>
//class Queue
//{
//public:
//    /**
//     * Constructor
//     *
//     * @param size Capacity of the queue (pre-allocated)
//     */
//    Queue(uint32_t size) :
//        m_size(size),
//        m_queue(size)
//    {
//
//    }
//
//    /**
//     * Destructor
//     */
//    ~Queue(void)
//    {
//
//    }
//
//    /**
//     * Add an element to the back of the queue
//     *
//     * @param elem Element to add
//     * @return True if successful, false if adding failed
//     *          (most likely due to full queue)
//     */
//    bool PushBack(_T&& elem)
//    {
//        return m_queue.try_enqueue(elem);
//    }
//
//    /**
//     * Add an element to the back of the queue
//     *
//     * @param elem Element to add
//     * @return True if successful, false if adding failed
//     *          (most likely due to full queue)
//     */
//    bool PushBack(const _T& elem)
//    {
//        return m_queue.try_enqueue(elem);
//    }
//
//    /**
//     * Remove an element from the front of the queue
//     * @param elem Reference to return the contents of the removed element to
//     * @return True if removing from the front was sucessfull, false if failed
//     *          (most likely due to empty queue)
//     */
//    bool PopFront(_T& elem)
//    {
//        return m_queue.try_dequeue(elem);
//    }
//
//    /**
//     * Get the number of elements currently in the queue
//     *
//     * @note This value might be inaccurate due to the lock free nature
//     * @return Number of elements in the queue
//     */
//    uint32_t GetElementCount(void) const {
//        return m_queue.size_approx();
//    }
//
//    /**
//     * Clear the queue (remove all elements)
//     */
//    void Clear(void)
//    {
//        _T dummy;
//        uint8_t empty = 0;
//
//        // poll queue empty, might return false even if not empty
//        // ensure by polling a few more times
//        while (empty < 10) {
//            if (!m_queue.try_dequeue(dummy)) {
//                empty++;
//            } else {
//                empty = 0;
//            }
//        }
//    }
//
//private:
//    const uint32_t m_size;
//
//    moodycamel::ConcurrentQueue<_T> m_queue;
//};
//
//}
//}

// ----------------------------------------------------------------------------

#include <atomic>
#include <mutex>
#include <thread>

namespace ibnet {
namespace sys {

template<typename _T>
class Queue
{
public:
    Queue(uint32_t size) :
        m_size(size),
        m_data((_T*) malloc(sizeof(_T) * m_size)),
        m_front(0),
        m_back(0)
    {

    }

    ~Queue(void)
    {
        free(m_data);
    }

    bool PushBack(_T&& elem)
    {
        m_mutex.lock();

        if ((m_back + 1) % m_size == m_front % m_size) {
            m_mutex.unlock();
            return false;
        }

        std::swap(m_data[m_back % m_size], elem);
        m_back++;

        m_mutex.unlock();
        return true;
    }

    bool PushBack(const _T& elem)
    {
        m_mutex.lock();

        if ((m_back + 1) % m_size == m_front % m_size) {
            m_mutex.unlock();
            return false;
        }

        m_data[m_back % m_size] = elem;
        m_back++;

        m_mutex.unlock();
        return true;
    }

    bool PopFront(_T& elem)
    {
        m_mutex.lock();

        if (m_back % m_size == m_front % m_size) {
            m_mutex.unlock();
            return false;
        }

        std::swap(elem, m_data[m_front % m_size]);
        m_front++;

        m_mutex.unlock();

        return true;
    }

    uint32_t GetElementCount(void) const {
        uint32_t tmp;

        uint32_t back = m_back;
        uint32_t front = m_front;

        if (back >= front) {
            tmp = back - front;
        } else {
            tmp = front - back;
        }

        return tmp;
    }

    void Clear(void) {
        m_mutex.lock();

        m_front = 0;
        m_back = 0;

        m_mutex.unlock();
    }

    void Print(void) {
        m_mutex.lock();

        uint32_t len;

        if (m_back >= m_front) {
            len = m_back - m_front;
        } else {
            len = m_front - m_back;
        }

        for (uint32_t i = 0; i < len; i++) {
            uint32_t pos = m_front + i;

            if (pos >= m_size) {
                pos -= m_size;
            }

            std::cout << m_data << ", ";
        }

        std::cout << std::endl;

        m_mutex.unlock();
    }

private:
    const uint32_t m_size;
    _T* m_data;

    std::mutex m_mutex;
    uint32_t m_front;
    uint32_t m_back;
};

}
}

#endif //IBNET_SYS_QUEUE_H
