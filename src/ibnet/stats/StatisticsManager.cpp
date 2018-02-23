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

#include "StatisticsManager.h"

namespace ibnet {
namespace stats {

StatisticsManager::StatisticsManager(uint32_t printIntervalMs) :
    m_printIntervalMs(printIntervalMs),
    m_mutex(),
    m_operations()
{
#ifdef IBNET_DISABLE_STATISTICS
    IBNET_LOG_INFO("Preprocessor flag to disable some statistics active");
#endif
}

void StatisticsManager::PrintStatistics()
{
    std::cout << "================= Statistics =================" << std::endl;

#ifdef IBNET_DISABLE_STATISTICS
    std::cout << "DISABLED DISABLED DISABLED DISABLED DISABLED" << std::endl;
#endif

    m_mutex.lock();

    for (auto& it : m_operations) {
        std::cout << *it << std::endl;
    }

    m_mutex.unlock();
}

void StatisticsManager::_RunLoop()
{
    _Sleep(m_printIntervalMs);
    PrintStatistics();
}

}
}