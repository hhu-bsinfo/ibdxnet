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
#include <ibnet/core/IbDevice.h>

#include "ibnet/Config.h"
#include "ibnet/sys/ThreadLoop.h"

#include "Operation.hpp"
#include "Unit.hpp"
#include "Throughput.hpp"


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
    explicit StatisticsManager(uint32_t printIntervalMs, ibnet::core::IbDevice* refDevice);

    /**
     * Destructor
     */
    ~StatisticsManager() override;

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
    void RefreshPerformanceCounters();

private:
    const uint32_t m_printIntervalMs;

    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<const Operation*>> m_operations;

    IbPerfLib::IbPerfCounter *m_perfCounter;
    IbPerfLib::IbDiagPerfCounter *m_diagPerfCounter;

private:
    Time* m_totalTime;

    /* Performance counters */
    Unit* m_rawXmitData;
    Unit* m_rawRcvData;
    Unit* m_rawXmitPkts;
    Unit* m_rawRcvPkts;

    Unit* m_unicastXmitPkts;
    Unit* m_unicastRcvPkts;
    Unit* m_multicastXmitPkts;
    Unit* m_multicastRcvPkts;

    Unit* m_symbolErrors;
    Unit* m_linkDowned;
    Unit* m_linkRecoveries;
    Unit* m_rcvErrors;
    Unit* m_rcvRemotePhysicalErrors;
    Unit* m_rcvSwitchRelayErrors;
    Unit* m_xmitDiscards;
    Unit* m_xmitConstraintErrors;
    Unit* m_rcvConstraintErrors;
    Unit* m_localLinkIntegrityErrors;
    Unit* m_excessiveBufferOverrunErrors;
    Unit* m_vl15Dropped;
    Unit* m_xmitWait;

    Throughput* m_rawXmitThroughput;
    Throughput* m_rawRcvThroughput;

    /* Diagnostic performance counters */
    Unit* m_lifespan;

    Unit* m_rqLocalLengthErrors;
    Unit* m_rqLocalProtectionErrors;
    Unit* m_rqLocalQpProtectionErrors;
    Unit* m_rqOutOfSequenceErrors;
    Unit* m_rqRemoteAccessErrors;
    Unit* m_rqRemoteInvalidRequestErrors;
    Unit* m_rqRnrNakNum;
    Unit* m_rqCompletionQueueEntryErrors;

    Unit* m_sqBadResponseErrors;
    Unit* m_sqLocalLengthErrors;
    Unit* m_sqLocalProtectionErrors;
    Unit* m_sqLocalQpProtectionErrors;
    Unit* m_sqMemoryWindowBindErrors;
    Unit* m_sqOutOfSequenceErrors;
    Unit* m_sqRemoteAccessErrors;
    Unit* m_sqRemoteInvalidRequestErrors;
    Unit* m_sqRnrNakNum;
    Unit* m_sqRemoteOperationErrors;
    Unit* m_sqRnrNakRetriesExceededErrors;
    Unit* m_sqTransportRetriesExceededErrors;
    Unit* m_sqCompletionQueueEntryErrors;
};

}
}

#endif //IBNET_DX_STATISTICSMANAGER_H
