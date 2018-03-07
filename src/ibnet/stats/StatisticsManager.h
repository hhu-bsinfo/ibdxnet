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
#include <unordered_map>
#include <vector>

#include "ibnet/Config.h"
#include "ibnet/sys/ThreadLoop.h"

#include "Operation.hpp"

namespace ibnet {
namespace stats {

/**
 * Manager for statistic operations. A dedicated thread prints all registered
 * statistics periodically if enabled.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.02.2018
 */
class StatisticsManager : public sys::ThreadLoop
{
public:
    /**
     * Constructor
     *
     * @param printIntervalMs Interval in ms to print all registered
     *        statistics (0 to disable printing)
     */
    explicit StatisticsManager(uint32_t printIntervalMs);

    /**
     * Destructor
     */
    ~StatisticsManager() = default;

    /**
     * Register a statistic operation
     *
     * @param refOperation Operation to register (caller has to manage memory)
     */
    void Register(const Operation* refOperation);

    /**
     * Deregister an already registered operation
     *
     * Ensure to call this before deleting the operation
     *
     * @param refOperation Operation to deregister
     */
    void Deregister(const Operation* refOperation);

    /**
     * Print the current state of all registerted statistics to stdout
     */
    void PrintStatistics();

protected:
    void _RunLoop() override;

private:
    const uint32_t m_printIntervalMs;

    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<const Operation*>> m_operations;
};

}
}

#endif //IBNET_DX_STATISTICSMANAGER_H
