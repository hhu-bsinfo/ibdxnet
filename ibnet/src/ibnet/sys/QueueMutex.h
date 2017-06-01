#ifndef IBNET_SYS_QUEUE_H
#define IBNET_SYS_QUEUE_H

#include <mutex>

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
        m_back(0)
    {

    }

    ~Queue(void)
    {
        free(m_data);
    }

    bool PushBack(_T* elem)
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

    _T* PopFront(void)
    {
        _T* elem = nullptr;

        m_mutex.lock();

        if (m_back % m_size == m_front % m_size) {
            m_mutex.unlock();
            return elem;
        }

        elem = m_data[m_front % m_size];
        m_front++;

        m_mutex.unlock();

        return elem;
    }

private:
    const uint32_t m_size;
    _T** m_data;

    std::mutex m_mutex;
    uint32_t m_front;
    uint32_t m_back;
};

}
}

#endif //IBNET_SYS_QUEUE_H
