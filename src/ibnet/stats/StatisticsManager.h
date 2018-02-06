//
// Created by nothaas on 2/1/18.
//

#ifndef IBNET_DX_STATISTICSMANAGER_H
#define IBNET_DX_STATISTICSMANAGER_H

#include <mutex>
#include <vector>

#include "ibnet/sys/ThreadLoop.h"

#include "Operation.hpp"

namespace ibnet {
namespace stats {

class StatisticsManager : public sys::ThreadLoop
{
public:
    explicit StatisticsManager(uint32_t printIntervalMs);

    ~StatisticsManager() = default;

    void Register(const Operation* operation) {
        std::lock_guard<std::mutex> l(m_mutex);
        m_operations.push_back(operation);
    }

    void Deregister(const Operation* operation) {
        std::lock_guard<std::mutex> l(m_mutex);

        for (auto it = m_operations.begin(); it != m_operations.end(); it++) {
            if (*it == operation) {
                m_operations.erase(it);
            }
        }
    }

    void PrintStatistics();

protected:
    void _RunLoop() override;

private:
    const uint32_t m_printIntervalMs;

    std::mutex m_mutex;
    std::vector<const Operation*> m_operations;
};

}
}

#endif //IBNET_DX_STATISTICSMANAGER_H
