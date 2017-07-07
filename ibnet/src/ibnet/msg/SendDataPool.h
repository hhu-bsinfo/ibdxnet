#ifndef IBNET_MSG_SENDDATAPOOL_H
#define IBNET_MSG_SENDDATAPOOL_H

#include <atomic>

#include "SendData.h"

namespace ibnet {
namespace msg {

class SendDataPool
{
public:
    SendDataPool(uint32_t size) :
        m_size(size),
        m_pool(new SendData*[m_size]),
        m_availPos(0),
        m_retPos(-1)
    {
        for (uint32_t i = 0; i < m_size; i++) {
            m_pool[i] = new SendData();
        }
    }

    ~SendDataPool(void)
    {
        for (uint32_t i = 0; i < m_size; i++) {
            delete m_pool[i];
        }

        delete m_pool;
    }

    SendData* Get(void)
    {
        uint32_t idx = m_pos.load(std::memory_order_relaxed);
        SendData* ret;

        do {
            if (idx >= m_size) {
                return NULL;
            }

            ret = m_pool[idx];
        } while (!m_availPos.compare_exchange_weak(idx, idx + 1,
            std::memory_order_acquire));

        m_retPos.fetch_add(1, std::memory_order_relaxed);

        return ret;
    }

    void Return(SendData* data)
    {
        uint32_t idxRet = m_retPos.fetch_sub(1, std::memory_order_relaxed);

        do {


            if (idxRet - 1 == -1) {
                throw std::runtime_error(
                    "SendDataPool: illegal state, underflow");
            }

            m_pool[idxRet] = data;

            uint32_t expected = idxRet + 1;

            while (!m_availPos.compare_exchange_weak(expected, idxRet,
                std::memory_order_release)) {
                // reset expected because we have to wait until we reach that pos
                expected = idxRet + 1;
                std::this_thread::yield();
            }
        } while (true);
    }

private:
    const uint32_t m_size;
    SendData** m_pool;

    std::atomic<uint32_t> m_availPos;
    std::atomic<uint32_t> m_retPos;
};

}
}

#endif //IBNET_MSG_SENDDATAPOOL_H
