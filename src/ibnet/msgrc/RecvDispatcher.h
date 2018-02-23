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

#ifndef IBNET_DX_MSGRCRECVDISPATCHER_H
#define IBNET_DX_MSGRCRECVDISPATCHER_H

#include "ibnet/dx/ExecutionUnit.h"
#include "ibnet/dx/RecvBufferPool.h"

#include "ibnet/stats/StatisticsManager.h"
#include "ibnet/stats/Throughput.hpp"

#include "ConnectionManager.h"
#include "RecvHandler.h"

namespace ibnet {
namespace msgrc {

//
// Created by nothaas on 1/30/18.
//
class RecvDispatcher : public dx::ExecutionUnit
{
public:
    RecvDispatcher(ConnectionManager* refConnectionManager,
        dx::RecvBufferPool* refRecvBufferPool,
        stats::StatisticsManager* refStatisticsManager,
        RecvHandler* refRecvHandler);

    ~RecvDispatcher() override;

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

    stats::Unit* m_receivedData;
    stats::Unit* m_receivedFC;

    stats::Throughput* m_throughputReceivedData;
    stats::Throughput* m_throughputReceivedFC;

//    stats::Time m_recievedData;
//    stats::Time m_recievedFC;

    // TODO statistics
//    uint64_t m_recvBytes;
//    uint64_t m_recvCounter;
//    uint64_t m_recvChunks;
//    uint64_t m_recvFlowControl;
//    uint64_t m_recvStallsCount;
//
//    uint64_t m_pollCompCounter;
//
//    sys::Time m_timer;
//    sys::Time m_timer2;
//    sys::Time m_timer3;
//    sys::Time m_timer4;
//
//    std::chrono::high_resolution_clock::time_point m_start;
};

}
}

#endif //IBNET_DX_MSGRCRECVDISPATCHER_H
