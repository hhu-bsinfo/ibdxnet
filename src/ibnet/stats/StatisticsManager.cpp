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
        m_diagPerfCounter(refDevice->GetDiagPerfCounter()),
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
        m_rawRcvThroughput(new Throughput("PerformanceCounters", "RcvThroughput", m_rawRcvData, m_totalTime)),
        m_lifespan(new Unit("DiagnosticPerformanceCounters", "Lifespan")),
        m_rqLocalLengthErrors(new Unit("DiagnosticPerformanceCounters", "RqLocalLengthErrors")),
        m_rqLocalProtectionErrors(new Unit("DiagnosticPerformanceCounters", "RqLocalProtectionErrors")),
        m_rqLocalQpProtectionErrors(new Unit("DiagnosticPerformanceCounters", "RqLocalQpProtectionErrors")),
        m_rqOutOfSequenceErrors(new Unit("DiagnosticPerformanceCounters", "RqOutOfSequenceErrors")),
        m_rqRemoteAccessErrors(new Unit("DiagnosticPerformanceCounters", "RqRemoteAccessErrors")),
        m_rqRemoteInvalidRequestErrors(new Unit("DiagnosticPerformanceCounters", "RqRemoteInvalidRequestErrors")),
        m_rqRnrNakNum(new Unit("DiagnosticPerformanceCounters", "RqRnrNakNum")),
        m_rqCompletionQueueEntryErrors(new Unit("DiagnosticPerformanceCounters", "RqCompletionQueueEntryErrors")),
        m_sqBadResponseErrors(new Unit("DiagnosticPerformanceCounters", "SqBadResponseErrors")),
        m_sqLocalLengthErrors(new Unit("DiagnosticPerformanceCounters", "SqLocalLengthErrors")),
        m_sqLocalProtectionErrors(new Unit("DiagnosticPerformanceCounters", "SqLocalProtectionErrors")),
        m_sqLocalQpProtectionErrors(new Unit("DiagnosticPerformanceCounters", "SqLocalQpProtectionErrors")),
        m_sqMemoryWindowBindErrors(new Unit("DiagnosticPerformanceCounters", "SqMemoryWindowBindErrors")),
        m_sqOutOfSequenceErrors(new Unit("DiagnosticPerformanceCounters", "SqOutOfSequenceErrors")),
        m_sqRemoteAccessErrors(new Unit("DiagnosticPerformanceCounters", "SqRemoteAccessErrors")),
        m_sqRemoteInvalidRequestErrors(new Unit("DiagnosticPerformanceCounters", "SqRemoteInvalidRequestErrors")),
        m_sqRnrNakNum(new Unit("DiagnosticPerformanceCounters", "SqRnrNakNum")),
        m_sqRemoteOperationErrors(new Unit("DiagnosticPerformanceCounters", "SqRemoteOperationErrors")),
        m_sqRnrNakRetriesExceededErrors(new Unit("DiagnosticPerformanceCounters", "SqRnrNakRetriesExceededErrors")),
        m_sqTransportRetriesExceededErrors(
                new Unit("DiagnosticPerformanceCounters", "SqTransportRetriesExceededErrors")),
        m_sqCompletionQueueEntryErrors(new Unit("DiagnosticPerformanceCounters", "SqCompletionQueueEntryErrors"))
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

    Register(m_lifespan);

    Register(m_rqLocalLengthErrors);
    Register(m_rqLocalProtectionErrors);
    Register(m_rqLocalQpProtectionErrors);
    Register(m_rqOutOfSequenceErrors);
    Register(m_rqRemoteAccessErrors);
    Register(m_rqRemoteInvalidRequestErrors);
    Register(m_rqRnrNakNum);
    Register(m_rqCompletionQueueEntryErrors);

    Register(m_sqBadResponseErrors);
    Register(m_sqLocalLengthErrors);
    Register(m_sqLocalProtectionErrors);
    Register(m_sqLocalQpProtectionErrors);
    Register(m_sqMemoryWindowBindErrors);
    Register(m_sqOutOfSequenceErrors);
    Register(m_sqRemoteAccessErrors);
    Register(m_sqRemoteInvalidRequestErrors);
    Register(m_sqRnrNakNum);
    Register(m_sqRemoteOperationErrors);
    Register(m_sqRnrNakRetriesExceededErrors);
    Register(m_sqTransportRetriesExceededErrors);
    Register(m_sqCompletionQueueEntryErrors);

    if (m_perfCounter) {
        m_perfCounter->ResetCounters();
    }

    if (m_diagPerfCounter) {
        m_diagPerfCounter->ResetCounters();
    }

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

    Deregister(m_lifespan);

    Deregister(m_rqLocalLengthErrors);
    Deregister(m_rqLocalProtectionErrors);
    Deregister(m_rqLocalQpProtectionErrors);
    Deregister(m_rqOutOfSequenceErrors);
    Deregister(m_rqRemoteAccessErrors);
    Deregister(m_rqRemoteInvalidRequestErrors);
    Deregister(m_rqRnrNakNum);
    Deregister(m_rqCompletionQueueEntryErrors);

    Deregister(m_sqBadResponseErrors);
    Deregister(m_sqLocalLengthErrors);
    Deregister(m_sqLocalProtectionErrors);
    Deregister(m_sqLocalQpProtectionErrors);
    Deregister(m_sqMemoryWindowBindErrors);
    Deregister(m_sqOutOfSequenceErrors);
    Deregister(m_sqRemoteAccessErrors);
    Deregister(m_sqRemoteInvalidRequestErrors);
    Deregister(m_sqRnrNakNum);
    Deregister(m_sqRemoteOperationErrors);
    Deregister(m_sqRnrNakRetriesExceededErrors);
    Deregister(m_sqTransportRetriesExceededErrors);
    Deregister(m_sqCompletionQueueEntryErrors);

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

    delete m_lifespan;

    delete m_rqLocalLengthErrors;
    delete m_rqLocalProtectionErrors;
    delete m_rqLocalQpProtectionErrors;
    delete m_rqOutOfSequenceErrors;
    delete m_rqRemoteAccessErrors;
    delete m_rqRemoteInvalidRequestErrors;
    delete m_rqRnrNakNum;
    delete m_rqCompletionQueueEntryErrors;

    delete m_sqBadResponseErrors;
    delete m_sqLocalLengthErrors;
    delete m_sqLocalProtectionErrors;
    delete m_sqLocalQpProtectionErrors;
    delete m_sqMemoryWindowBindErrors;
    delete m_sqOutOfSequenceErrors;
    delete m_sqRemoteAccessErrors;
    delete m_sqRemoteInvalidRequestErrors;
    delete m_sqRnrNakNum;
    delete m_sqRemoteOperationErrors;
    delete m_sqRnrNakRetriesExceededErrors;
    delete m_sqTransportRetriesExceededErrors;
    delete m_sqCompletionQueueEntryErrors;
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

    RefreshPerformanceCounters();

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

void StatisticsManager::RefreshPerformanceCounters() {
    try {
        if (m_perfCounter) {
            m_perfCounter->RefreshCounters();
        }

        if (m_diagPerfCounter) {
            m_diagPerfCounter->RefreshCounters();
        }
    } catch(IbPerfLib::IbPerfException &exception) {
        IBNET_LOG_WARN("Failed to query performance counters! Error: %s", exception.what());
    }

    if (m_perfCounter) {
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
    }

    if (m_diagPerfCounter) {
        IBNET_STATS(m_lifespan->Add(m_diagPerfCounter->GetLifespan() - m_lifespan->GetTotalValue()));

        IBNET_STATS(m_rqLocalLengthErrors->Add(
                m_diagPerfCounter->GetRqLocalLengthErrors() - m_rqLocalLengthErrors->GetTotalValue()));
        IBNET_STATS(m_rqLocalQpProtectionErrors->Add(
                m_diagPerfCounter->GetRqLocalQpProtectionErrors() - m_rqLocalQpProtectionErrors->GetTotalValue()));
        IBNET_STATS(m_rqOutOfSequenceErrors->Add(
                m_diagPerfCounter->GetRqOutOfSequenceErrors() - m_rqOutOfSequenceErrors->GetTotalValue()));
        IBNET_STATS(m_rqRemoteAccessErrors->Add(
                m_diagPerfCounter->GetRqRemoteAccessErrors() - m_rqRemoteAccessErrors->GetTotalValue()));
        IBNET_STATS(m_rqRemoteInvalidRequestErrors->Add(
                m_diagPerfCounter->GetRqRemoteInvalidRequestErrors() - m_rqRemoteInvalidRequestErrors->GetTotalValue()));
        IBNET_STATS(m_rqRnrNakNum->Add(
                m_diagPerfCounter->GetRqRnrNakNum() - m_rqRnrNakNum->GetTotalValue()));
        IBNET_STATS(m_rqCompletionQueueEntryErrors->Add(
                m_diagPerfCounter->GetRqCompletionQueueEntryErrors() - m_rqCompletionQueueEntryErrors->GetTotalValue()));

        IBNET_STATS(m_sqBadResponseErrors->Add(
                m_diagPerfCounter->GetSqBadResponseErrors() - m_sqBadResponseErrors->GetTotalValue()));
        IBNET_STATS(m_sqLocalLengthErrors->Add(
                m_diagPerfCounter->GetSqLocalLengthErrors() - m_sqLocalLengthErrors->GetTotalValue()));
        IBNET_STATS(m_sqLocalProtectionErrors->Add(
                m_diagPerfCounter->GetSqLocalProtectionErrors() - m_sqLocalProtectionErrors->GetTotalValue()));
        IBNET_STATS(m_sqLocalQpProtectionErrors->Add(
                m_diagPerfCounter->GetSqLocalQpProtectionErrors() - m_sqLocalQpProtectionErrors->GetTotalValue()));
        IBNET_STATS(m_sqMemoryWindowBindErrors->Add(
                m_diagPerfCounter->GetSqMemoryWindowBindErrors() - m_sqMemoryWindowBindErrors->GetTotalValue()));
        IBNET_STATS(m_sqOutOfSequenceErrors->Add(
                m_diagPerfCounter->GetSqOutOfSequenceErrors() - m_sqOutOfSequenceErrors->GetTotalValue()));
        IBNET_STATS(m_sqRemoteAccessErrors->Add(
                m_diagPerfCounter->GetSqRemoteAccessErrors() - m_sqRemoteAccessErrors->GetTotalValue()));
        IBNET_STATS(m_sqRemoteInvalidRequestErrors->Add(
                m_diagPerfCounter->GetSqRemoteInvalidRequestErrors() - m_sqRemoteInvalidRequestErrors->GetTotalValue()));
        IBNET_STATS(m_sqRnrNakNum->Add(m_diagPerfCounter->GetSqRnrNakNum() - m_sqRnrNakNum->GetTotalValue()));
        IBNET_STATS(m_sqRemoteOperationErrors->Add(
                m_diagPerfCounter->GetSqRemoteOperationErrors() - m_sqRemoteOperationErrors->GetTotalValue()));
        IBNET_STATS(m_sqRnrNakRetriesExceededErrors->Add(
                m_diagPerfCounter->GetSqRnrNakRetriesExceededErrors() - m_sqRnrNakRetriesExceededErrors->GetTotalValue()));
        IBNET_STATS(m_sqTransportRetriesExceededErrors->Add(
                m_diagPerfCounter->GetSqTransportRetriesExceededErrors() -
                m_sqTransportRetriesExceededErrors->GetTotalValue()));
        IBNET_STATS(m_sqCompletionQueueEntryErrors->Add(
                m_diagPerfCounter->GetSqCompletionQueueEntryErrors() - m_sqCompletionQueueEntryErrors->GetTotalValue()));
    }

    IBNET_STATS(m_totalTime->Stop());
    IBNET_STATS(m_totalTime->Start());
}

}
}