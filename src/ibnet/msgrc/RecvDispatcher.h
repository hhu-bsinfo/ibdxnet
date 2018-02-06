//
// Created by nothaas on 1/30/18.
//

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
