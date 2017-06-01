#ifndef IBNET_SYS_QUEUE_H
#define IBNET_SYS_QUEUE_H

#include <atomic>

namespace ibnet {
namespace sys {

template<typename _T>
class Queue
{
public:
    Queue(uint32_t size) :
        m_size(size),
        m_data((_T**) malloc(sizeof(_T*) * m_size)),
        m_front(0),
        m_back(0),
        m_lock(false)
    {

    }

    ~Queue(void)
    {
        free(m_data);
    }

    bool PushBack(_T* elem)
    {
        __Lock();

        if ((m_back + 1) % m_size == m_front % m_size) {
            __Unlock();
            return false;
        }

        m_data[m_back % m_size] = elem;
        m_back++;

        __Unlock();
        return true;
    }

    _T* PopFront(void)
    {
        _T* elem = nullptr;

        __Lock();

        if (m_back % m_size == m_front % m_size) {
            __Unlock();
            return elem;
        }

        elem = m_data[m_front % m_size];
        m_front++;

        __Unlock();

        return elem;
    }

private:
    const uint32_t m_size;
    _T** m_data;

    uint32_t m_front;
    uint32_t m_back;

    std::atomic<bool> m_lock;

    inline void __Lock(void) {
        bool exp = false;

        while (!m_lock.compare_exchange_weak(exp, true)) {
            exp = false;
            std::this_thread::yield();
        }
    }

    inline void __Unlock(void) {
        m_lock.store(false);
    }
};

}
}

#endif //IBNET_SYS_QUEUE_H
