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
#include "IbPerfLib/Exception/IbPerfException.h"

namespace ibnet {
namespace stats {

StatisticsManager::StatisticsManager(uint32_t printIntervalMs, ibnet::core::IbDevice* refDevice) :
        m_printIntervalMs(printIntervalMs),
        m_mutex(),
        m_operations(),
        m_perfCounter(refDevice->GetPerfCounter()),
        m_totalTime(new Time("PerformanceCounters", "TotalTime")),
        m_rawXmitData(new Unit("PerformanceCounters", "XmitData")),
        m_rawRcvData(new Unit("PerformanceCounters", "RcvData")),
        m_rawXmitPkts(new Unit("PerformanceCounters", "XmitPkts")),
        m_rawRcvPkts(new Unit("PerformanceCounters", "RcvPkts")),
        m_unicastXmitPkts(new Unit("PerformanceCounters", "UnicastXmitPkts")),
        m_unicastRcvPkts(new Unit("PerformanceCounters", "UnicastRcvPkts")),
        m_multicastXmitPkts(new Unit("PerformanceCounters", "MulitcastXmitPkts")),
        m_multicastRcvPkts(new Unit("PerformanceCounters", "MulitcastRcvPkts")),
        m_symbolErrors(new Unit("PerformanceCounters", "SymbolErrors")),
        m_linkDowned(new Unit("PerformanceCounters", "LinkDowned")),
        m_linkRecoveries(new Unit("PerformanceCounters", "LinkRecoveries")),
        m_rcvErrors(new Unit("PerformanceCounters", "RcvErrors")),
        m_rcvRemotePhysicalErrors(new Unit("PerformanceCounters", "RcvRemotePhysicalErrors")),
        m_rcvSwitchRelayErrors(new Unit("PerformanceCounters", "RcvSwitchRelayErrors")),
        m_xmitDiscards(new Unit("PerformanceCounters", "XmitDiscards")),
        m_xmitConstraintErrors(new Unit("PerformanceCounters", "XmitConstraintErrors")),
        m_rcvConstraintErrors(new Unit("PerformanceCounters", "RcvConstraintErrors")),
        m_localLinkIntegrityErrors(new Unit("PerformanceCounters", "LocalLinkIntegrityErrors")),
        m_excessiveBufferOverrunErrors(new Unit("PerformanceCounters", "ExcessiveBufferOverrunErrors")),
        m_vl15Dropped(new Unit("PerformanceCounters", "VL15Dropped")),
        m_xmitWait(new Unit("PerformanceCounters", "XmitWait")),
        m_rawXmitThroughput(new Throughput("PerformanceCounters", "XmitThroughput", m_rawXmitData, m_totalTime)),
        m_rawRcvThroughput(new Throughput("PerformanceCounters", "RcvThroughput", m_rawRcvData, m_totalTime))
{
#ifdef IBNET_DISABLE_STATISTICS
    IBNET_LOG_INFO("Preprocessor flag to disable some statistics active");
#endif

    Register(m_rawXmitData);
    Register(m_rawRcvData);
    Register(m_rawXmitPkts);
    Register(m_rawRcvPkts);

    Register(m_unicastXmitPkts);
    Register(m_unicastRcvPkts);
    Register(m_multicastXmitPkts);
    Register(m_multicastRcvPkts);

    Register(m_symbolErrors);
    Register(m_linkDowned);
    Register(m_linkRecoveries);
    Register(m_rcvErrors);
    Register(m_rcvRemotePhysicalErrors);
    Register(m_rcvSwitchRelayErrors);
    Register(m_xmitDiscards);
    Register(m_xmitConstraintErrors);
    Register(m_rcvConstraintErrors);
    Register(m_localLinkIntegrityErrors);
    Register(m_excessiveBufferOverrunErrors);
    Register(m_vl15Dropped);
    Register(m_xmitWait);

    Register(m_rawXmitThroughput);
    Register(m_rawRcvThroughput);

    m_perfCounter->ResetCounters();

    IBNET_STATS(m_totalTime->Start());
}

StatisticsManager::~StatisticsManager() {
    Deregister(m_rawXmitData);
    Deregister(m_rawRcvData);
    Deregister(m_rawXmitPkts);
    Deregister(m_rawRcvPkts);

    Deregister(m_unicastXmitPkts);
    Deregister(m_unicastRcvPkts);
    Deregister(m_multicastXmitPkts);
    Deregister(m_multicastRcvPkts);

    Deregister(m_symbolErrors);
    Deregister(m_linkDowned);
    Deregister(m_linkRecoveries);
    Deregister(m_rcvErrors);
    Deregister(m_rcvRemotePhysicalErrors);
    Deregister(m_rcvSwitchRelayErrors);
    Deregister(m_xmitDiscards);
    Deregister(m_xmitConstraintErrors);
    Deregister(m_rcvConstraintErrors);
    Deregister(m_localLinkIntegrityErrors);
    Deregister(m_excessiveBufferOverrunErrors);
    Deregister(m_vl15Dropped);
    Deregister(m_xmitWait);

    Deregister(m_rawXmitThroughput);
    Deregister(m_rawRcvThroughput);

    delete m_rawXmitData;
    delete m_rawRcvData;
    delete m_rawXmitPkts;
    delete m_rawRcvPkts;

    delete m_unicastXmitPkts;
    delete m_unicastRcvPkts;
    delete m_multicastXmitPkts;
    delete m_multicastRcvPkts;

    delete m_symbolErrors;
    delete m_linkDowned;
    delete m_linkRecoveries;
    delete m_rcvErrors;
    delete m_rcvRemotePhysicalErrors;
    delete m_rcvSwitchRelayErrors;
    delete m_xmitDiscards;
    delete m_xmitConstraintErrors;
    delete m_rcvConstraintErrors;
    delete m_localLinkIntegrityErrors;
    delete m_excessiveBufferOverrunErrors;
    delete m_vl15Dropped;
    delete m_xmitWait;

    delete m_rawXmitThroughput;
    delete m_rawRcvThroughput;
}


void StatisticsManager::Register(const Operation* refOperation)
{
    std::lock_guard<std::mutex> l(m_mutex);

    auto it = m_operations.find(refOperation->GetCategoryName());

    if (it == m_operations.end()) {
        m_operations.insert(std::make_pair(refOperation->GetCategoryName(),
                std::vector<const Operation*>({refOperation})));
    } else {
        it->second.push_back(refOperation);
    }
}

void StatisticsManager::Deregister(const Operation* refOperation)
{
    std::lock_guard<std::mutex> l(m_mutex);

    auto it = m_operations.find(refOperation->GetCategoryName());

    if (it != m_operations.end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            if (*it2 == refOperation) {
                it->second.erase(it2);
                break;
            }
        }
    }
}

void StatisticsManager::PrintStatistics()
{
    std::cout << "================= Statistics =================" << std::endl;

#ifdef IBNET_DISABLE_STATISTICS
    std::cout << "DISABLED DISABLED DISABLED DISABLED DISABLED" << std::endl;
#endif

    std::stringstream sstr;

    m_mutex.lock();

    try {
        m_perfCounter->RefreshCounters();
    } catch(IbPerfLib::IbPerfException e) {
        IBNET_LOG_WARN("Failed to query performance counters! Error: %s", e.what());
    }

    IBNET_STATS(m_rawXmitData->Add(m_perfCounter->GetXmitDataBytes() - m_rawXmitData->GetTotalValue()));
    IBNET_STATS(m_rawRcvData->Add(m_perfCounter->GetRcvDataBytes() - m_rawRcvData->GetTotalValue()));
    IBNET_STATS(m_rawXmitPkts->Add(m_perfCounter->GetXmitPkts() - m_rawXmitPkts->GetTotalValue()));
    IBNET_STATS(m_rawRcvPkts->Add(m_perfCounter->GetRcvPkts() - m_rawRcvPkts->GetTotalValue()));

    IBNET_STATS(m_unicastXmitPkts->Add(m_perfCounter->GetUnicastXmitPkts() - m_unicastXmitPkts->GetTotalValue()));
    IBNET_STATS(m_unicastRcvPkts->Add(m_perfCounter->GetUnicastRcvPkts() - m_unicastRcvPkts->GetTotalValue()));
    IBNET_STATS(m_multicastXmitPkts->Add(m_perfCounter->GetMulticastXmitPkts() - m_multicastXmitPkts->GetTotalValue()));
    IBNET_STATS(m_multicastRcvPkts->Add(m_perfCounter->GetMulticastRcvPkts() - m_multicastRcvPkts->GetTotalValue()));

    IBNET_STATS(m_symbolErrors->Add(m_perfCounter->GetSymbolErrors() - m_symbolErrors->GetTotalValue()));
    IBNET_STATS(m_linkDowned->Add(m_perfCounter->GetLinkDownedCounter() - m_linkDowned->GetTotalValue()));
    IBNET_STATS(m_linkRecoveries->Add(m_perfCounter->GetLinkRecoveryCounter() - m_linkRecoveries->GetTotalValue()));
    IBNET_STATS(m_rcvErrors->Add(m_perfCounter->GetRcvErrors() - m_rcvErrors->GetTotalValue()));
    IBNET_STATS(m_rcvRemotePhysicalErrors->Add(
            m_perfCounter->GetRcvRemotePhysicalErrors() -m_rcvRemotePhysicalErrors->GetTotalValue()));
    IBNET_STATS(m_rcvSwitchRelayErrors->Add(
            m_perfCounter->GetRcvSwitchRelayErrors() - m_rcvSwitchRelayErrors->GetTotalValue()));
    IBNET_STATS(m_xmitDiscards->Add(m_perfCounter->GetXmitDiscards() - m_xmitDiscards->GetTotalValue()));
    IBNET_STATS(m_xmitConstraintErrors->Add(
            m_perfCounter->GetXmitConstraintErrors() - m_xmitConstraintErrors->GetTotalValue()));
    IBNET_STATS(m_rcvConstraintErrors->Add(
            m_perfCounter->GetRcvConstraintErrors() - m_rcvConstraintErrors->GetTotalValue()));
    IBNET_STATS(m_localLinkIntegrityErrors->Add(
            m_perfCounter->GetLocalLinkIntegrityErrors() - m_localLinkIntegrityErrors->GetTotalValue()));
    IBNET_STATS(m_excessiveBufferOverrunErrors->Add(
            m_perfCounter->GetExcessiveBufferOverrunErrors() - m_excessiveBufferOverrunErrors->GetTotalValue()));
    IBNET_STATS(m_vl15Dropped->Add(m_perfCounter->GetVL15Dropped() - m_vl15Dropped->GetTotalValue()));
    IBNET_STATS(m_xmitWait->Add(m_perfCounter->GetXmitWait() - m_xmitWait->GetTotalValue()));

    IBNET_STATS(m_totalTime->Stop());
    IBNET_STATS(m_totalTime->Start());


    for (auto& it : m_operations) {
        sstr << ">>> " << it.first << std::endl;

        for (auto& it2 : it.second) {
            sstr << *it2 << std::endl;
        }
    }

    m_mutex.unlock();

    std::cout << sstr.str();
}

void StatisticsManager::_RunLoop()
{
    _Sleep(m_printIntervalMs);
    PrintStatistics();
}

}
}