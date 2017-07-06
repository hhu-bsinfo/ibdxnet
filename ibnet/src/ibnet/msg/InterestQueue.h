#ifndef IBNET_MSG_INTERESTQUEUE_H
#define IBNET_MSG_INTERESTQUEUE_H

#include <atomic>
#include <mutex>
#include <thread>

namespace ibnet {
namespace msg {

class InterestQueue
{
public:
    InterestQueue(uint32_t size) :
        m_size(size),
        m_data((uint16_t*) malloc(sizeof(uint16_t) * m_size)),
        m_front(0),
        m_back(0)
    {

    }

    ~InterestQueue(void)
    {
        free(m_data);
    }

    bool PushBack(uint16_t val)
    {
        m_mutex.lock();

        if ((m_back + 1) % m_size == m_front % m_size) {
            m_mutex.unlock();
            return false;
        }

        m_data[m_back % m_size] = val;
        m_back++;

        m_mutex.unlock();
        return true;
    }

    bool PopFront(uint16_t& elem)
    {
        m_mutex.lock();

        if (m_back % m_size == m_front % m_size) {
            m_mutex.unlock();
            return false;
        }

        elem = m_data[m_front % m_size];
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

            std::cout << std::hex << m_data << ", ";
        }

        std::cout << std::endl;

        m_mutex.unlock();
    }

private:
    const uint32_t m_size;
    uint16_t* m_data;

    std::mutex m_mutex;
    uint32_t m_front;
    uint32_t m_back;
};

}
}

#endif //IBNET_SYS_QUEUE_H
