/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_DX_STATISTICSMANAGER_H
#define IBNET_DX_STATISTICSMANAGER_H

#include <mutex>
#include <vector>

#include "ibnet/sys/ThreadLoop.h"

#include "Operation.hpp"

//#define IBNET_DISABLE_STATISTICS
#ifdef IBNET_DISABLE_STATISTICS
#define IBNET_STATS(...)
#else
#define IBNET_STATS(x) x
#endif

namespace ibnet {
namespace stats {

//
// Created by nothaas on 2/1/18.
//
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
                break;
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
