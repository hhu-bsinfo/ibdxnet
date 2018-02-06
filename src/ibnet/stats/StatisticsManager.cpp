//
// Created by nothaas on 2/1/18.
//

#include "StatisticsManager.h"

namespace ibnet {
namespace stats {

StatisticsManager::StatisticsManager(uint32_t printIntervalMs) :
    m_printIntervalMs(printIntervalMs),
    m_mutex(),
    m_operations()
{

}

void StatisticsManager::PrintStatistics()
{
    std::cout << "================= Statistics =================" << std::endl;

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