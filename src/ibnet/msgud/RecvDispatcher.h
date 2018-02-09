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

#ifndef IBNET_DX_MSGUDRECVDISPATCHER_H
#define IBNET_DX_MSGUDRECVDISPATCHER_H

#include "ibnet/dx/ExecutionUnit.h"
#include "ibnet/dx/RecvBufferPool.h"

#include "ibnet/stats/StatisticsManager.h"
#include "ibnet/stats/Throughput.hpp"
#include "ibnet/stats/TimelineFragmented.hpp"

#include "ConnectionManager.h"
#include "RecvHandler.h"

namespace ibnet {
namespace msgud {

/**
 * Execution unit dispatching incoming data for the UD messaging subsystem
 *
 * Based on msgrc/RecvDispatcher by Stefan Nothaas.
 *
 * @author Fabian Ruhland, fabian.ruhland@hhu.de, 08.02.2018
 */
class RecvDispatcher : public dx::ExecutionUnit
{
public:
    /**
     * Constructor
     *
     * @param refConnectionManager Pointer to the connection manager (managed by caller)
     * @param refRecvBufferPool Pointer to the receive buffer pool used for incoming data (managed by caller)
     * @param refStatisticsManager Pointer to the statistics manager (managed by caller)
     * @param refRecvHandler Pointer to the receive handler to dispatch the received data to (managed by caller)
     */
    RecvDispatcher(ConnectionManager* refConnectionManager,
        dx::RecvBufferPool* refRecvBufferPool,
        stats::StatisticsManager* refStatisticsManager,
        RecvHandler* refRecvHandler);

    /**
     * Destructor
     */
    ~RecvDispatcher() override;

    /**
     * Overring virtual function
     */
    bool Dispatch() override;

private:
    ConnectionManager* m_refConnectionManager;
    dx::RecvBufferPool* m_refRecvBufferPool;
    stats::StatisticsManager* m_refStatisticsManager;
    RecvHandler* m_refRecvHandler;

private:
    RecvHandler::ReceivedPackage* m_recvPackage;

    uint16_t m_recvQueuePending;
    ibv_wc* m_workComps;
    bool m_firstWc;

    core::IbMemReg** m_memRegRefillBuffer;
    ibv_sge* m_sgeList;
    ibv_recv_wr* m_recvWrList;

private:
    uint32_t __Poll();

    void __ProcessReceived(uint32_t receivedCount);

    void __Refill();

    template<typename ExceptionType, typename... Args>
    void __ThrowDetailedException(const std::string& reason, Args... args) {
        throw ExceptionType(reason + "\n"
            "SendDispatcher state:\n"
            "m_recvQueuePending: %d\n"
            "m_totalTime: %s\n"
            "m_receivedData: %s\n"
            "m_receivedFC: %s\n"
            "m_throughputReceivedData: %s\n"
            "m_throughputReceivedFC: %s"
            , args...,
            m_recvQueuePending,
            *m_totalTime,
            *m_receivedData,
            *m_receivedFC,
            *m_throughputReceivedData,
            *m_throughputReceivedFC);
    };

    template<typename ExceptionType, typename... Args>
    void __ThrowDetailedException(int ret, const std::string& reason,
            Args... args) {
        __ThrowDetailedException<ExceptionType>(reason + "\nError (%d): %s",
            args..., ret, strerror(ret));
    };

private:
    stats::Time* m_totalTime;

    stats::Time* m_pollTime;
    stats::Time* m_processRecvTotalTime;
    stats::Time* m_processRecvAvailTime;
    stats::Time* m_processRecvHandleTime;
    stats::Time* m_refillTotalTime;
    stats::Time* m_refillAvailTime;
    stats::Time* m_refillGetBuffersTime;
    stats::Time* m_refillPostTime;
    stats::Time* m_eeSchedTime;

    stats::TimelineFragmented* m_recvTimeline;
    stats::TimelineFragmented* m_processRecvTimeline;
    stats::TimelineFragmented* m_refillTimeline;

    stats::Unit* m_receivedData;
    stats::Unit* m_receivedFC;

    stats::Throughput* m_throughputReceivedData;
    stats::Throughput* m_throughputReceivedFC;
};

}
}

#endif