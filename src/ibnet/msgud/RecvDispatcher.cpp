//
// Created by ruhland on 2/9/18.
//

#include "RecvDispatcher.h"

#include "ibnet/core/IbCommon.h"
#include "ibnet/core/IbQueueFullException.h"

#include "ibnet/con/DisconnectedException.h"

#include "Common.h"

namespace ibnet {
namespace msgud {

RecvDispatcher::RecvDispatcher(ConnectionManager* refConnectionManager,
    dx::RecvBufferPool* refRecvBufferPool,
    stats::StatisticsManager* refStatisticsManager,
    RecvHandler* refRecvHandler) :
    ExecutionUnit("MsgUDRecv"),
    m_refConnectionManager(refConnectionManager),
    m_refRecvBufferPool(refRecvBufferPool),
    m_refStatisticsManager(refStatisticsManager),
    m_refRecvHandler(refRecvHandler),
    m_recvPackage(static_cast<RecvHandler::ReceivedPackage*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            RecvHandler::ReceivedPackage::Sizeof(
                refConnectionManager->GetIbQPSize())))),
    m_recvQueuePending(0),
    m_workComps(static_cast<ibv_wc*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(ibv_wc) * refConnectionManager->GetIbQPSize()))),
    m_firstWc(true),
    m_memRegRefillBuffer(static_cast<core::IbMemReg**>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(core::IbMemReg*) * refConnectionManager->GetIbQPSize()))),
    m_sgeList(static_cast<ibv_sge*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(ibv_sge) * refConnectionManager->GetIbQPSize()))),
    m_recvWrList(static_cast<ibv_recv_wr*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(ibv_recv_wr) * refConnectionManager->GetIbQPSize()))),
    m_totalTime(new stats::Time("RecvDispatcher", "Total")),
    m_pollTime(new stats::Time("RecvDispatcher", "Poll")),
    m_processRecvTotalTime(new stats::Time("RecvDispatcher", "ProcessRecvTotal")),
    m_processRecvAvailTime(new stats::Time("RecvDispatcher", "ProcessRecvAvail")),
    m_processRecvHandleTime(new stats::Time("RecvDispatcher", "ProcessRecvHandle")),
    m_refillTotalTime(new stats::Time("RecvDispatcher", "RefillTotal")),
    m_refillAvailTime(new stats::Time("RecvDispatcher", "RefillAvail")),
    m_refillGetBuffersTime(new stats::Time("RecvDispatcher", "RefillGetBuffers")),
    m_refillPostTime(new stats::Time("RecvDispatcher", "RefillPost")),
    m_eeSchedTime(new stats::Time("RecvDispatcher", "EESched")),
    m_recvTimeline(new stats::TimelineFragmented("RecvDispatcher", "Recv", m_totalTime, {m_pollTime,
        m_processRecvTotalTime, m_refillTotalTime, m_eeSchedTime})),
    m_processRecvTimeline(new stats::TimelineFragmented("RecvDispatcher", "ProcessRecv", m_processRecvTotalTime,
        {m_processRecvAvailTime, m_processRecvHandleTime})),
    m_refillTimeline(new stats::TimelineFragmented("RecvDispatcher", "Refill", m_refillTotalTime, {m_refillAvailTime,
        m_refillGetBuffersTime, m_refillPostTime})),
    m_receivedData(new stats::Unit("RecvDispatcher", "Data", stats::Unit::e_Base2)),
    m_receivedFC(new stats::Unit("RecvDispatcher", "FC", stats::Unit::e_Base10)),
    m_throughputReceivedData(new stats::Throughput("RecvDispatcher", "ThroughputData",
        m_receivedData, m_totalTime)),
    m_throughputReceivedFC(new stats::Throughput("RecvDispatcher", "ThroughputFC",
        m_receivedFC, m_totalTime))
{
    memset(m_recvPackage, 0, RecvHandler::ReceivedPackage::Sizeof(
        refConnectionManager->GetIbQPSize()));
    memset(m_workComps, 0,
        sizeof(ibv_wc) * refConnectionManager->GetIbQPSize());
    memset(m_memRegRefillBuffer, 0,
        sizeof(core::IbMemReg*) * refConnectionManager->GetIbQPSize());
    memset(m_sgeList, 0,
        sizeof(core::IbMemReg*) * refConnectionManager->GetIbQPSize());
    memset(m_recvWrList, 0,
        sizeof(ibv_sge) * refConnectionManager->GetIbQPSize());

    m_refStatisticsManager->Register(m_totalTime);
    m_refStatisticsManager->Register(m_recvTimeline);
    m_refStatisticsManager->Register(m_processRecvTimeline);
    m_refStatisticsManager->Register(m_refillTimeline);
    m_refStatisticsManager->Register(m_receivedData);
    m_refStatisticsManager->Register(m_receivedFC);
    m_refStatisticsManager->Register(m_throughputReceivedData);
    m_refStatisticsManager->Register(m_throughputReceivedFC);
}    

RecvDispatcher::~RecvDispatcher()
{
    free(m_recvPackage);

    m_refStatisticsManager->Deregister(m_totalTime);
    m_refStatisticsManager->Deregister(m_recvTimeline);
    m_refStatisticsManager->Deregister(m_processRecvTimeline);
    m_refStatisticsManager->Deregister(m_refillTimeline);
    m_refStatisticsManager->Deregister(m_receivedData);
    m_refStatisticsManager->Deregister(m_receivedFC);
    m_refStatisticsManager->Deregister(m_throughputReceivedData);
    m_refStatisticsManager->Deregister(m_throughputReceivedFC);

    free(m_workComps);
    free(m_memRegRefillBuffer);
    free(m_sgeList);
    free(m_recvWrList);

    delete m_totalTime;

    delete m_pollTime;
    delete m_processRecvTotalTime;
    delete m_processRecvAvailTime;
    delete m_processRecvHandleTime;
    delete m_refillTotalTime;
    delete m_refillAvailTime;
    delete m_refillGetBuffersTime;
    delete m_refillPostTime;
    delete m_eeSchedTime;

    delete m_recvTimeline;
    delete m_processRecvTimeline;
    delete m_refillTimeline;

    delete m_receivedData;
    delete m_receivedFC;
    delete m_throughputReceivedData;
    delete m_throughputReceivedFC;
}

bool RecvDispatcher::Dispatch()
{
    IBNET_STATS(m_eeSchedTime->Stop());

    if (m_totalTime->GetCounter() == 0) {
        IBNET_STATS(m_totalTime->Start());
    } else {
        IBNET_STATS(m_totalTime->Stop());
        IBNET_STATS(m_totalTime->Start());
    }

    IBNET_STATS(m_pollTime->Start());

    uint32_t receivedCount = __Poll();

    IBNET_STATS(m_pollTime->Stop());
    IBNET_STATS(m_processRecvTotalTime->Start());

    __ProcessReceived(receivedCount);

    IBNET_STATS(m_processRecvTotalTime->Stop());
    IBNET_STATS(m_refillTotalTime->Start());

    __Refill();

    IBNET_STATS(m_refillTotalTime->Stop());
    IBNET_STATS(m_eeSchedTime->Start());

    return receivedCount > 0;
}

uint32_t RecvDispatcher::__Poll()
{
    // poll in batches to reduce overhead and increase utilization
    int ret = ibv_poll_cq(m_refConnectionManager->GetIbRecvCQ(),
        m_refConnectionManager->GetIbCQSize(), m_workComps);
    
    if(ret < 0) {
        __ThrowDetailedException<core::IbException>(
            ret, "Polling completion queue failed");
    }

    auto receivedCount = static_cast<uint32_t>(ret);

    // polling successful, iterate work completions and check for errors
    for(uint32_t i = 0; i < receivedCount; i++) {
        if(m_workComps[i].status != IBV_WC_SUCCESS) {
            if(m_workComps[i].status) {
                switch(m_workComps[i].status) {
                    // a previous work request failed and put the queue into error
                    // state

                    case IBV_WC_RETRY_EXC_ERR:
                        if(m_firstWc) {
                            __ThrowDetailedException<core::IbException>(
                                "First work completion of queue failed, it's "
                                "very likely your connection attributes are "
                                "wrong or the remote site isn't in a state to "
                                "respond");
                        } else {
                            throw con::DisconnectedException();
                        }
                    default:
                        __ThrowDetailedException<core::IbException>(
                            "Found failed work completion (%d), status %s",
                            i, core::WORK_COMPLETION_STATUS_CODE[
                            m_workComps[i].status]);
                }
            }
        }

        m_firstWc = false;
    }

    m_recvQueuePending -= receivedCount;

    return receivedCount;
}

void RecvDispatcher::__ProcessReceived(uint32_t receivedCount)
{
    if(receivedCount > 0) {
        IBNET_STATS(m_processRecvAvailTime->Start());

        for(uint32_t i = 0; i < receivedCount; i++) {
            auto* immedData = (ImmediateData*) &m_workComps[i].imm_data;
            auto* dataMem = (core::IbMemReg*) m_workComps[i].wr_id;
            uint32_t dataRecvLen = m_workComps[i].byte_len;

            // evaluate data
            // check for zero length package which can't be indicated by setting
            // the size to 0 on send (which gets translated to 2^31 instead)
            if (immedData->m_zeroLengthData) {
                dataRecvLen = 0;

                // return buffer to pool, don't care about any dummy data
                // otherwise, the pool runs dry after a while
                m_refRecvBufferPool->ReturnBuffer(dataMem);
                dataMem = nullptr;

                if (!immedData->m_flowControlData) {
                    __ThrowDetailedException<sys::IllegalStateException>(
                        "Zero length data received but no flow control data");
                }
            }

            if (immedData->m_flowControlData) {
                IBNET_STATS(m_receivedFC->Inc());
            }

            m_recvPackage->m_entries[i].m_sourceNodeId =
                immedData->m_sourceNodeId;
            m_recvPackage->m_entries[i].m_fcData = immedData->m_flowControlData;
            m_recvPackage->m_entries[i].m_data = dataMem;
            m_recvPackage->m_entries[i].m_dataRaw =
                dataMem ? dataMem->GetAddress() : nullptr;
            m_recvPackage->m_entries[i].m_dataLength = dataRecvLen;

            IBNET_STATS(m_receivedData->Add(dataRecvLen));
        }

        m_recvPackage->m_count = receivedCount;

        IBNET_STATS(m_processRecvHandleTime->Start());

        // buffers are returned to recv buffer pool async
        m_refRecvHandler->Received(m_recvPackage);

        IBNET_STATS(m_processRecvHandleTime->Stop());

        m_recvPackage->m_count = 0;

        IBNET_STATS(m_processRecvAvailTime->Stop());
    }
}

void RecvDispatcher::__Refill()
{
    if(m_recvQueuePending < m_refConnectionManager->GetIbQPSize()) {
        IBNET_STATS(m_refillAvailTime->Start());

        uint32_t count = m_refConnectionManager->GetIbQPSize() - m_recvQueuePending;

        IBNET_STATS(m_refillGetBuffersTime->Start());

        uint32_t numBufs = m_refRecvBufferPool->GetBuffers(m_memRegRefillBuffer, count);

        IBNET_STATS(m_refillGetBuffersTime->Stop());

        // first failed work request
        ibv_recv_wr* bad_wr;

        for (uint32_t i = 0; i < numBufs; i++) {
            // hook memory to write the received data to
            m_sgeList[i].addr = (uintptr_t) m_memRegRefillBuffer[i]->GetAddress();
            m_sgeList[i].length = m_memRegRefillBuffer[i]->GetSize();
            m_sgeList[i].lkey = m_memRegRefillBuffer[i]->GetLKey();

            // Use the pointer as the work req id
            m_recvWrList[i].wr_id = (uint64_t) m_memRegRefillBuffer[i];
            m_recvWrList[i].sg_list = &m_sgeList[i];
            m_recvWrList[i].num_sge = 1;
            m_recvWrList[i].next = nullptr;
        }

        // chain work requests
        for (uint32_t i = 0; i < numBufs - 1; i++) {
            m_recvWrList[i].next = &m_recvWrList[i + 1];
        }

        IBNET_STATS(m_refillPostTime->Start());

        int ret = ibv_post_recv(m_refConnectionManager->GetIbQP(),
            &m_recvWrList[0], &bad_wr);

        IBNET_STATS(m_refillPostTime->Stop());
        
        if (ret != 0) {
            switch (ret) {
                case ENOMEM:
                    __ThrowDetailedException<core::IbQueueFullException>(
                        "Receive queue full");

                default:
                    __ThrowDetailedException<core::IbException>(ret,
                        "Posting work request to receive queue failed, numBufs "
                            "%d", numBufs);
            }
        }

        m_recvQueuePending += numBufs;

        IBNET_STATS(m_refillAvailTime->Stop());
    }
}

}
}