#ifndef PROJECT_IBQUEUETRACKER_H
#define PROJECT_IBQUEUETRACKER_H

#include <atomic>
#include <cstdbool>
#include <cstdint>

#include "IbException.h"

namespace ibnet {
namespace core {

class IbQueueTracker
{
public:
    IbQueueTracker(uint16_t size) :
        m_size(size),
        m_outstanding(0)
    {};

    ~IbQueueTracker(void) {};

    inline bool AddOutstanding(void) {
        uint16_t outstandingSends =
            m_outstanding.load(std::memory_order_relaxed);

        do {
            if (outstandingSends == m_size) {
                return false;
            }
        } while (!m_outstanding.compare_exchange_weak(outstandingSends,
            outstandingSends + 1, std::memory_order_relaxed));

        return true;
    }

    inline bool SubOutstanding(void) {
        uint16_t tmp = m_outstanding.load(std::memory_order_relaxed);

        do {
            if (tmp == 0) {
                return false;
            }
        } while (!m_outstanding.compare_exchange_weak(tmp, tmp - 1,
            std::memory_order_relaxed));

        return true;
    }

    inline uint16_t GetCurrent(void) const {
        return m_outstanding.load(std::memory_order_relaxed);
    }

private:
    const uint16_t m_size;
    std::atomic<uint16_t> m_outstanding;
};

}
}

#endif //PROJECT_IBQUEUETRACKER_H
