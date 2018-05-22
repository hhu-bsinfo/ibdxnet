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

#ifndef IBNET_DX_MSGRCSENDDISPATCHER_H
#define IBNET_DX_MSGRCSENDDISPATCHER_H

#include <sstream>

#include "ibnet/dx/ExecutionUnit.h"

#include "ibnet/stats/Distribution.hpp"
#include "ibnet/stats/StatisticsManager.h"
#include "ibnet/stats/Ratio.hpp"
#include "ibnet/stats/Throughput.hpp"
#include "ibnet/stats/TimelineFragmented.hpp"

#include "Connection.h"
#include "ConnectionManager.h"
#include "SendHandler.h"
#include "SendWorkRequestCtxPool.h"

namespace ibnet {
namespace msgrc {

/**
 * Execution unit getting new data to be sent from a SendHandler and
 * sending it to the specified target using reliable queue pairs and
 * messaging verbs.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 30.01.2018
 */
class SendDispatcher : public dx::ExecutionUnit
{
public:
    /**
     * Constructor
     *
     * @param recvBufferSize Size of a single receive buffer (from the RecvBufferPool)
     * @param refConnectionManager Pointer to the connection manager (memory managed by caller)
     * @param refStatisticsManager Pointer to the statistics manager (memory managed by caller)
     * @param refSendHandler Pointer to a send handler which provides data to be sent (memory managed by caller)
     */
    SendDispatcher(uint32_t recvBufferSize,
            ConnectionManager* refConnectionManager,
            stats::StatisticsManager* refStatisticsManager,
            SendHandler* refSendHandler);

    /**
     * Destructor
     */
    ~SendDispatcher() override;

    /**
     * Overriding virtual function
     */
    bool Dispatch() override;

private:
    const uint32_t m_recvBufferSize;

    ConnectionManager* m_refConnectionManager;
    stats::StatisticsManager* m_refStatisticsManager;
    SendHandler* m_refSendHandler;

private:
    SendHandler::PrevWorkPackageResults* m_prevWorkPackageResults;
    SendHandler::CompletedWorkList* m_completionList;

    uint32_t m_completionsPending;
    uint16_t m_sendQueuePending[con::NODE_ID_MAX_NUM_NODES];
    bool m_firstWc;
    uint32_t m_ignoreFlushErrOnPendingCompletions;

    ibv_sge* m_sgeLists;
    ibv_send_wr* m_sendWrs;
    ibv_wc* m_workComp;

    SendWorkRequestCtxPool* m_workRequestCtxPool;

    sys::Timer m_sendBlockTimer;

private:
    bool __PollCompletions();

    bool __SendData(Connection* connection, const SendHandler::NextWorkPackage* workPackage);

    uint32_t __SendDataPrepareWorkRequests(Connection* connection, const SendHandler::NextWorkPackage* workPackage);

    void __SendDataPostWorkRequests(Connection* connection, uint32_t chunks);

    void __DebugLogWorkReqList(uint32_t numElems);

    template <typename ExceptionType, typename... Args>
    void __ThrowDetailedException(const std::string& reason, Args... args)
    {
        std::stringstream sendQueuePending;

        for (uint16_t i = 0; i < con::NODE_ID_MAX_NUM_NODES; i++) {
            if (m_sendQueuePending[i] > 0) {
                sendQueuePending << std::hex << i << "|" << std::dec <<
                        m_sendQueuePending[i] << " ";
            }
        }

        throw ExceptionType(reason + "\n"
                        "SendDispatcher state:\n"
                        "m_prevWorkPackageResults: %s\n"
                        "m_completionList: %s\n"
                        "m_completionsPending: %s\n"
                        "m_sendQueuePending: %s\n"
                        "m_totalTime: %s\n"
                        "m_sentData: %s\n"
                        "m_sentFC: %s\n"
                        "m_throughputSentData: %s\n"
                        "m_throughputSentFC: %s", args...,
                *m_prevWorkPackageResults,
                *m_completionList,
                m_completionsPending,
                sendQueuePending.str(),
                *m_totalTime,
                *m_sentData,
                *m_sentFC,
                *m_throughputSentData,
                *m_throughputSentFC);
    };

    template <typename ExceptionType, typename... Args>
    void __ThrowDetailedException(int ret, const std::string& reason,
            Args... args)
    {
        __ThrowDetailedException<ExceptionType>(reason + "\nError (%d): %s\n%s",
                args..., ret, strerror(ret));
    };

private:
    class Stats : public stats::Operation
    {
    public:
        Stats(SendDispatcher* refParent) :
                Operation("SendDispatcher", "State"),
                m_refParent(refParent)
        {
        }

        ~Stats() override = default;

        void WriteOstream(std::ostream& os, const std::string& indent) const override
        {
            os << indent << "m_prevWorkPackageResults " <<
                    *m_refParent->m_prevWorkPackageResults << ", m_completionList "
                    << *m_refParent->m_completionList << ", m_completionsPending "
                    << m_refParent->m_completionsPending << ", m_sendQueuePending ";

            for (uint16_t i = 0; i < con::NODE_ID_MAX_NUM_NODES; i++) {
                if (m_refParent->m_sendQueuePending[i] > 0) {
                    os << std::hex << i << "|" << std::dec <<
                            m_refParent->m_sendQueuePending[i] << " ";
                }
            }
        }

    private:
        SendDispatcher* m_refParent;
    };

private:
    stats::Time* m_totalTime;

    stats::Time* m_getNextDataToSendTime;
    stats::Time* m_pollCompletionsTotalTime;
    stats::Time* m_pollCompletionsActiveTime;
    stats::Time* m_getConnectionTime;
    stats::Time* m_sendDataTotalTime;
    stats::Time* m_sendDataProcessingTime;
    stats::Time* m_sendDataPostingTime;
    stats::Time* m_eeScheduleTime;

    stats::TimelineFragmented* m_totalTimeline;
    stats::TimelineFragmented* m_pollTimeline;
    stats::TimelineFragmented* m_sendTimeline;

    stats::Unit* m_postedWRQs;
    stats::Unit* m_postedDataChunk;
    stats::Unit* m_postedDataRemainderChunk;
    stats::Distribution* m_sendType;

    stats::Unit* m_sentData;
    stats::Unit* m_sentFC;

    stats::Unit* m_sendBlock100ms;
    stats::Unit* m_sendBlock250ms;
    stats::Unit* m_sendBlock500ms;

    stats::Unit* m_emptyNextWorkPackage;
    stats::Unit* m_nonEmptyNextWorkPackage;

    stats::Unit* m_sendDataFullBuffers;
    stats::Unit* m_sendDataNonFullBuffers;
    stats::Unit* m_sendQueueFull;

    stats::Unit* m_emptyCompletionPolls;
    stats::Unit* m_nonEmptyCompletionPolls;
    stats::Unit* m_completionBatches;

    stats::Ratio* m_nextWorkPackageRatio;
    stats::Ratio* m_sendDataFullBuffersRatio;
    stats::Ratio* m_emptyCompletionPollsRatio;

    stats::Throughput* m_throughputSentData;
    stats::Throughput* m_throughputSentFC;

    Stats* m_privateStats;
};

}
}

#endif //IBNET_DX_MSGRCSENDDISPATCHER_H
