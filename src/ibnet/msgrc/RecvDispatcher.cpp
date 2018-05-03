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

#include "RecvDispatcher.h"

#include "ibnet/sys/IllegalStateException.h"

#include "ibnet/core/IbCommon.h"
#include "ibnet/core/IbQueueFullException.h"

#include "ibnet/con/DisconnectedException.h"

#include "Common.h"

namespace ibnet {
namespace msgrc {

RecvDispatcher::RecvDispatcher(ConnectionManager* refConnectionManager,
        dx::RecvBufferPool* refRecvBufferPool,
        stats::StatisticsManager* refStatisticsManager,
        RecvHandler* refRecvHandler) :
        ExecutionUnit("MsgRCRecv"),
        m_refConnectionManager(refConnectionManager),
        m_refRecvBufferPool(refRecvBufferPool),
        m_refStatisticsManager(refStatisticsManager),
        m_refRecvHandler(refRecvHandler),
        m_recvPackage(static_cast<RecvHandler::ReceivedPackage*>(
                aligned_alloc(static_cast<size_t>(getpagesize()),
                        RecvHandler::ReceivedPackage::Sizeof(
                                refConnectionManager->GetIbSRQSize() * refConnectionManager->GetMaxSGEs())))),
        m_recvQueuePending(0),
        // TODO make configurable?
        m_totalWorkComps(refConnectionManager->GetIbSRQSize()),
        m_workCompsProd(static_cast<ibv_wc*>(
                aligned_alloc(static_cast<size_t>(getpagesize()), sizeof(ibv_wc) * m_totalWorkComps))),
        m_workCompsProdPos(0),
        m_workCompsCons(static_cast<ibv_wc*>(
                aligned_alloc(static_cast<size_t>(getpagesize()), sizeof(ibv_wc) * m_totalWorkComps))),
        m_workCompsConsPos(0),
        m_firstWc(true),
        // TODO correct pool size for WRS with double buffering?
        m_recvWRPool(
                new RecvWorkRequestPool(refConnectionManager->GetIbSRQSize() * 2, refConnectionManager->GetMaxSGEs())),
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
        m_refillTimeline(
                new stats::TimelineFragmented("RecvDispatcher", "Refill", m_refillTotalTime, {m_refillAvailTime,
                        m_refillGetBuffersTime, m_refillPostTime})),
        m_postedWRQs(new stats::Unit("RecvDispatcher", "WRQsPosted", stats::Unit::e_Base10)),
        m_polledWRQs(new stats::Unit("RecvDispatcher", "WRQsPolled", stats::Unit::e_Base10)),
        m_receivedData(new stats::Unit("RecvDispatcher", "Data", stats::Unit::e_Base2)),
        m_receivedFC(new stats::Unit("RecvDispatcher", "FC", stats::Unit::e_Base10)),
        m_throughputReceivedData(new stats::Throughput("RecvDispatcher", "ThroughputData",
                m_receivedData, m_totalTime)),
        m_throughputReceivedFC(new stats::Throughput("RecvDispatcher", "ThroughputFC",
                m_receivedFC, m_totalTime)),
        m_privateStats(new Stats(this))
{
    memset(m_recvPackage, 0, RecvHandler::ReceivedPackage::Sizeof(refConnectionManager->GetIbSRQSize() *
            refConnectionManager->GetMaxSGEs()));
    memset(m_workCompsProd, 0, sizeof(ibv_wc) * m_totalWorkComps);
    memset(m_workCompsCons, 0, sizeof(ibv_wc) * m_totalWorkComps);

    m_refStatisticsManager->Register(m_totalTime);
    m_refStatisticsManager->Register(m_recvTimeline);
    m_refStatisticsManager->Register(m_processRecvTimeline);
    m_refStatisticsManager->Register(m_refillTimeline);
    m_refStatisticsManager->Register(m_postedWRQs);
    m_refStatisticsManager->Register(m_polledWRQs);
    m_refStatisticsManager->Register(m_receivedData);
    m_refStatisticsManager->Register(m_receivedFC);
    m_refStatisticsManager->Register(m_throughputReceivedData);
    m_refStatisticsManager->Register(m_throughputReceivedFC);

    m_refStatisticsManager->Register(m_privateStats);
}

RecvDispatcher::~RecvDispatcher()
{
    free(m_recvPackage);

    m_refStatisticsManager->Deregister(m_totalTime);
    m_refStatisticsManager->Deregister(m_recvTimeline);
    m_refStatisticsManager->Deregister(m_processRecvTimeline);
    m_refStatisticsManager->Deregister(m_refillTimeline);
    m_refStatisticsManager->Deregister(m_postedWRQs);
    m_refStatisticsManager->Deregister(m_polledWRQs);
    m_refStatisticsManager->Deregister(m_receivedData);
    m_refStatisticsManager->Deregister(m_receivedFC);
    m_refStatisticsManager->Deregister(m_throughputReceivedData);
    m_refStatisticsManager->Deregister(m_throughputReceivedFC);

    m_refStatisticsManager->Deregister(m_privateStats);

    free(m_workCompsProd);
    free(m_workCompsCons);

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

    delete m_postedWRQs;
    delete m_polledWRQs;
    delete m_receivedData;
    delete m_receivedFC;
    delete m_throughputReceivedData;
    delete m_throughputReceivedFC;

    delete m_privateStats;
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

    bool activity;

    IBNET_STATS(m_pollTime->Start());

    activity = __Poll();

    IBNET_STATS(m_pollTime->Stop());
    IBNET_STATS(m_refillTotalTime->Start());

    activity = __Refill() || activity;

    IBNET_STATS(m_refillTotalTime->Stop());
    IBNET_STATS(m_processRecvTotalTime->Start());

    activity = __ProcessReceived() || activity;

    IBNET_STATS(m_processRecvTotalTime->Stop());
    IBNET_STATS(m_eeSchedTime->Start());

    return activity;
}

bool RecvDispatcher::__Poll()
{
    if (m_totalWorkComps - m_workCompsProdPos > 0) {
        // poll in batches to reduce overhead and increase utilization
        int ret = ibv_poll_cq(m_refConnectionManager->GetIbSharedRCQ(), m_totalWorkComps - m_workCompsProdPos,
                &m_workCompsProd[m_workCompsProdPos]);

        if (ret < 0) {
            __ThrowDetailedException<core::IbException>(ret, "Polling completion queue failed");
        }

        auto receivedCount = static_cast<uint32_t>(ret);

        IBNET_STATS(m_polledWRQs->Add(receivedCount));

        // polling successful, iterate work completions and check for errors
        for (uint32_t i = 0; i < receivedCount; i++) {
            if (m_workCompsProd[m_workCompsProdPos + i].status != IBV_WC_SUCCESS) {
                if (m_workCompsProd[m_workCompsProdPos + i].status) {
                    switch (m_workCompsProd[m_workCompsProdPos + i].status) {
                        // a previous work request failed and put the queue into error
                        // state
                        //case IBV_WC_WR_FLUSH_ERR:
                        //    throw IbException("Work completion of recv queue failed, "
                        //        "flush err");

                        case IBV_WC_RETRY_EXC_ERR:
                            if (m_firstWc) {
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
                                    "Found failed work completion (%d), status %s", i,
                                    core::WORK_COMPLETION_STATUS_CODE[m_workCompsProd[m_workCompsProdPos + i].status]);

                    }
                }
            }

            m_firstWc = false;
        }

        m_workCompsProdPos += receivedCount;
        m_recvQueuePending -= receivedCount;
    }

    // space in comp list and work requests pending -> activity
    return m_totalWorkComps - m_workCompsProdPos > 0 && m_recvQueuePending > 0;
}

bool RecvDispatcher::__ProcessReceived()
{
    bool activity;

    while (true) {
        // still work completions left to consume
        if (m_recvPackage->m_count > 0) {
            IBNET_STATS(m_processRecvHandleTime->Start());

            // buffers are returned to recv buffer pool async
            uint32_t processed = m_refRecvHandler->Received(m_recvPackage);

            IBNET_STATS(m_processRecvHandleTime->Stop());

            if (processed == 0) {
                // queue full, set activity to false to allow parking
                activity = false;
            } else if (processed == m_recvPackage->m_count) {
                // all processed, continue with double buffer swap and next available data
                m_recvPackage->m_count = 0;

                activity = true;
            } else {
                // not all completions could be processed, shift in work package and try again later

                // null fc data which is processed before data but data could not be processed because the IBQ
                // was full. avoid processing already processed data again
                m_recvPackage->m_entries[processed - 1].m_fcData = 0;

                m_recvPackage->m_offset += processed;
                m_recvPackage->m_count -= processed;

                activity = true;
            }

            break;
        } else if (m_recvPackage->m_count == 0 && m_workCompsProdPos > 0) {
            // double buffer swap
            m_workCompsConsPos = m_workCompsProdPos;
            m_workCompsProdPos = 0;

            ibv_wc* tmp = m_workCompsProd;
            m_workCompsProd = m_workCompsCons;
            m_workCompsCons = tmp;

            // create receive package for procesisng
            IBNET_STATS(m_processRecvAvailTime->Start());

            uint32_t posRecvEntry = 0;

            // batch process all completions. one completion might contain multiple SGEs
            for (uint32_t i = 0; i < m_workCompsConsPos; i++) {
                auto* immedData = (ImmediateData*) &m_workCompsCons[i].imm_data;
                auto* recvWorkReq = (RecvWorkRequest*) m_workCompsCons[i].wr_id;
                uint32_t dataRecvLen = m_workCompsCons[i].byte_len;

                // evaluate data
                // we might receive 0 bytes which indicates that flow control only data was sent
                // and no SGEs were used on the remote sender
                if (dataRecvLen == 0) {
                    // SGE buffers are unused, return them to pool
                    m_refRecvBufferPool->ReturnBuffers(recvWorkReq->m_sgls.m_refsMemReg,
                            recvWorkReq->m_sgls.m_numUsedElems);

                    m_recvWRPool->Push(recvWorkReq);

                    // sanity check
                    if (!immedData->m_flowControlData) {
                        __ThrowDetailedException<sys::IllegalStateException>(
                                "Zero length data received but no flow control data");
                    }

                    // process flow control data once and add it to a single recv package
                    m_recvPackage->m_entries[posRecvEntry].m_sourceNodeId = immedData->m_sourceNodeId;
                    m_recvPackage->m_entries[posRecvEntry].m_fcData = immedData->m_flowControlData;
                    m_recvPackage->m_entries[posRecvEntry].m_data = nullptr;
                    m_recvPackage->m_entries[posRecvEntry].m_dataRaw = nullptr;
                    m_recvPackage->m_entries[posRecvEntry].m_dataLength = 0;

                    immedData->m_flowControlData = 0;
                    IBNET_STATS(m_receivedFC->Inc());

                    posRecvEntry++;
                } else {
                    uint32_t dataRecvLenTmp = dataRecvLen;
                    uint32_t sgesUsed = 0;

                    // if multiple SGEs were provided on recv post, we have to figure out which buffer contains
                    // received data using the total size
                    for (uint32_t j = 0; j < recvWorkReq->m_sgls.m_numUsedElems; j++) {
                        m_recvPackage->m_entries[posRecvEntry].m_sourceNodeId = immedData->m_sourceNodeId;

                        // fc data with immediate data available
                        if (immedData->m_flowControlData) {
                            m_recvPackage->m_entries[posRecvEntry].m_fcData = immedData->m_flowControlData;
                            immedData->m_flowControlData = 0;
                            IBNET_STATS(m_receivedFC->Inc());
                        } else {
                            m_recvPackage->m_entries[posRecvEntry].m_fcData = 0;
                        }

                        m_recvPackage->m_entries[posRecvEntry].m_data = recvWorkReq->m_sgls.m_refsMemReg[j];
                        m_recvPackage->m_entries[posRecvEntry].m_dataRaw =
                                recvWorkReq->m_sgls.m_refsMemReg[j]->GetAddress();

                        uint32_t maxBufferSize = recvWorkReq->m_sgls.m_refsMemReg[j]->GetSizeBuffer();

                        // figure out how much data is in the scattered buffers
                        if (dataRecvLenTmp >= maxBufferSize) {
                            m_recvPackage->m_entries[posRecvEntry].m_dataLength = maxBufferSize;
                            dataRecvLenTmp -= maxBufferSize;
                        } else {
                            m_recvPackage->m_entries[posRecvEntry].m_dataLength = dataRecvLenTmp;
                            dataRecvLenTmp = 0;
                        }

                        posRecvEntry++;
                        sgesUsed++;

                        if (dataRecvLenTmp == 0) {
                            break;
                        }
                    }

                    // return unused SGEs
                    m_refRecvBufferPool->ReturnBuffers(recvWorkReq->m_sgls.m_refsMemReg + sgesUsed,
                            recvWorkReq->m_sgls.m_numUsedElems - sgesUsed);

                    m_recvWRPool->Push(recvWorkReq);

                    IBNET_STATS(m_receivedData->Add(dataRecvLen));
                }
            }

            m_recvPackage->m_count = posRecvEntry;
            m_recvPackage->m_offset = 0;

            IBNET_STATS(m_processRecvAvailTime->Stop());
        } else {
            // nothing to process in consumer, nothing available to swap in producer
            activity = false;
            break;
        }
    }

    return activity;
}

bool RecvDispatcher::__Refill()
{
    if (m_recvQueuePending < m_refConnectionManager->GetIbSRQSize()) {
        IBNET_STATS(m_refillAvailTime->Start());

        uint32_t toQueueElems = m_refConnectionManager->GetIbSRQSize() - m_recvQueuePending;

        IBNET_STATS(m_refillGetBuffersTime->Start());

        RecvWorkRequest* recvWRs[toQueueElems];
        core::IbMemReg* refsMemReg[toQueueElems * m_refConnectionManager->GetMaxSGEs()];

        for (uint32_t i = 0; i < toQueueElems; i++) {
            recvWRs[i] = nullptr;
        }

        uint32_t numBufs = m_refRecvBufferPool->GetBuffers(&refsMemReg[0],
                toQueueElems * m_refConnectionManager->GetMaxSGEs());

        IBNET_STATS(m_refillGetBuffersTime->Stop());

        uint32_t queuedElems = 0;
        uint32_t buffersPos = 0;

        for (uint32_t i = 0; i < toQueueElems; i++) {
            // not enough buffers left to prepare work request with a full SGE list
            if (buffersPos + m_refConnectionManager->GetMaxSGEs() > numBufs) {
                break;
            }

            recvWRs[i] = m_recvWRPool->Pop();
            recvWRs[i]->m_sgls.Reset();

            // create SGE list, always with max length
            for (uint32_t j = 0; j < m_refConnectionManager->GetMaxSGEs(); j++) {
                recvWRs[i]->m_sgls.Add(refsMemReg[buffersPos++]);
            }

            recvWRs[i]->Prepare();
            queuedElems++;
        }

        // return unused buffers which could not fill a full SGE list
        if (buffersPos < numBufs) {
            m_refRecvBufferPool->ReturnBuffers(refsMemReg + buffersPos, numBufs - buffersPos);
        }

        // chain work requests
        if (queuedElems > 0) {
            for (uint32_t i = 0; i < queuedElems - 1; i++) {
                recvWRs[i]->Chain(recvWRs[i + 1]);
            }
        }

        // TODO track the batch sizes pulled from the completion queue and added back to the recv queue

        if (recvWRs[0]) {
            // first failed work request
            ibv_recv_wr* bad_wr;

            IBNET_STATS(m_refillPostTime->Start());

            int ret = ibv_post_srq_recv(m_refConnectionManager->GetIbSRQ(), &recvWRs[0]->m_recvWr, &bad_wr);

            IBNET_STATS(m_refillPostTime->Stop());

            if (ret != 0) {
                switch (ret) {
                    case ENOMEM:
                        __ThrowDetailedException<core::IbQueueFullException>("Receive queue full");

                    default: {
                        uint32_t idx = 0xFFFFFFFF;

                        // search for failed WRQ
                        for (uint32_t i = 0; i < queuedElems; i++) {
                            if (recvWRs[i]->m_recvWr.wr_id == bad_wr->wr_id) {
                                idx = i;
                                break;
                            }
                        }

                        __ThrowDetailedException<core::IbException>(ret, "Posting work request to receive queue "
                                "failed, num WRQs %d, first failed WRQ idx %d", queuedElems, idx);
                    }
                }
            }

            IBNET_STATS(m_postedWRQs->Add(queuedElems));

            m_recvQueuePending += queuedElems;
        }

        IBNET_STATS(m_refillAvailTime->Stop());

        return true;
    } else {
        return false;
    }
}

}
}