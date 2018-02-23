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

#include "ibnet/stats/StatisticsManager.h"
#include "ibnet/stats/Ratio.hpp"
#include "ibnet/stats/Throughput.hpp"

#include "Connection.h"
#include "ConnectionManager.h"
#include "SendHandler.h"

namespace ibnet {
namespace msgrc {

/**
 * Dedicated thread for sending data.
 *
 * The thread is using the SendHandler to call into the java space to get the
 * next buffer/data to send. If data is available, it fills one or, if enough
 * data is available, multiple pinned infiniband buffers and posts them to the
 * infiniband work queue. Afterwards, the same amount of work completions are
 * polled for optimal utilization. However, handling flow control data is
 * prioritized and always comes first to avoid deadlocking.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
// TODO update doc
//
// Created by nothaas on 1/30/18.
//
class SendDispatcher : public dx::ExecutionUnit
{
public:
    SendDispatcher(uint32_t recvBufferSize,
        ConnectionManager* refConnectionManager,
        stats::StatisticsManager* refStatisticsManager,
        SendHandler* refSendHandler);

    ~SendDispatcher() override;

    bool Dispatch() override;

private:
    struct WorkRequestIdCtx
    {
        con::NodeId m_targetNodeId;
        uint32_t m_sendSize;
        uint16_t m_fcData;
    } __attribute__((__packed__));

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

private:
    bool __PollCompletions();

    void __SendData(Connection* connection,
        const SendHandler::NextWorkPackage* workPackage);

    template<typename ExceptionType, typename... Args>
    void __ThrowDetailedException(const std::string& reason, Args... args) {
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
            "m_throughputSentFC: %s"
            , args...,
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

    template<typename ExceptionType, typename... Args>
    void __ThrowDetailedException(int ret, const std::string& reason,
            Args... args) {
        __ThrowDetailedException<ExceptionType>(reason + "\nError (%d): %s\n%s",
            args..., ret, strerror(ret));
    };

private:
    class Stats : public stats::Operation
    {
    public:
        Stats(SendDispatcher* refParent) :
            Operation("SendDispatcherState"),
            m_refParent(refParent)
        {}

        ~Stats() override = default;

        void WriteOstream(std::ostream& os) const override {
            os << "m_prevWorkPackageResults " <<
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
    stats::Time* m_pollCompletions;
    stats::Time* m_sendData;

    stats::Unit* m_sentData;
    stats::Unit* m_sentFC;

    stats::Unit* m_emptyNextWorkPackage;
    stats::Unit* m_nonEmptyNextWorkPackage;

    stats::Unit* m_sendDataFullBuffers;
    stats::Unit* m_sendDataNonFullBuffers;
    stats::Unit* m_sendBatches;

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
