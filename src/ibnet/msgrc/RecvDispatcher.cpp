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
        // TODO the ring buffer can hold more elements than just the IBQ size * SGEs -> make configurable?
        m_ringBuffer(new IncomingRingBuffer(refConnectionManager->GetIbSRQSize() * refConnectionManager->GetMaxSGEs())),
        m_workComps(static_cast<ibv_wc*>(
                aligned_alloc(static_cast<size_t>(getpagesize()), sizeof(ibv_wc) * refConnectionManager->GetIbSRQSize()))),
        m_received(0),
        m_recvQueuePending(0),
        m_firstWc(true),
        // TODO now with the IRB, this can be more than just what fits into the SRQ -> make configurable?
        m_recvWRPool(
                new RecvWorkRequestPool(refConnectionManager->GetIbSRQSize() * 2, refConnectionManager->GetMaxSGEs())),
        m_totalTime(new stats::Time("RecvDispatcher", "Total")),
        m_pollTime(new stats::Time("RecvDispatcher", "Poll")),
        m_processRecvTotalTime(new stats::Time("RecvDispatcher", "ProcessRecvTotal")),
        m_processRecvAvailTime(new stats::Time("RecvDispatcher", "ProcessRecvAvail")),
        m_processRecvHandleTime(new stats::Time("RecvDispatcher", "ProcessRecvHandle")),
        m_refillAvailTime(new stats::Time("RecvDispatcher", "RefillAvail")),
        m_refillGetBuffersTime(new stats::Time("RecvDispatcher", "RefillGetBuffers")),
        m_refillPostTime(new stats::Time("RecvDispatcher", "RefillPost")),
        m_eeSchedTime(new stats::Time("RecvDispatcher", "EESched")),
        m_postedWRQs(new stats::Unit("RecvDispatcher", "WRQsPosted", stats::Unit::e_Base10)),
        m_polledWRQs(new stats::Unit("RecvDispatcher", "WRQsPolled", stats::Unit::e_Base10)),
        m_queueEmptied(new stats::Unit("RecvDispatcher", "QueueEmptied", stats::Unit::e_Base10)),
        m_irbFull(new stats::Unit("RecvDispatcher", "IRBFull", stats::Unit::e_Base10)),
        m_receivedData(new stats::Unit("RecvDispatcher", "Data", stats::Unit::e_Base2)),
        m_receivedFC(new stats::Unit("RecvDispatcher", "FC", stats::Unit::e_Base10)),
        m_handlerNoProcess(new stats::Unit("RecvDispatcher", "HandlerNoProcess", stats::Unit::e_Base10)),
        m_refillInsufficientBuffers(new stats::Unit("RecvDispatcher", "RefillInsufficientBuffers",
                stats::Unit::e_Base10)),
        m_bufferUtilization(new stats::Ratio("RecvDispatcher", "BufferUtilization")),
        m_fragmentedLastBuffer(new stats::Ratio("RecvDispatcher", "FragmentedLastBuffer")),
        m_fragmentedSGEs(new stats::Ratio("RecvDispatcher", "FragmentedSGEs")),
        m_throughputReceivedData(new stats::Throughput("RecvDispatcher", "ThroughputData",
                m_receivedData, m_totalTime)),
        m_throughputReceivedFC(new stats::Throughput("RecvDispatcher", "ThroughputFC",
                m_receivedFC, m_totalTime)),
        m_privateStats(new Stats(this))
{
    m_refStatisticsManager->Register(m_totalTime);

    m_refStatisticsManager->Register(m_pollTime);
    m_refStatisticsManager->Register(m_processRecvTotalTime);
    m_refStatisticsManager->Register(m_processRecvAvailTime);
    m_refStatisticsManager->Register(m_processRecvHandleTime);
    m_refStatisticsManager->Register(m_refillAvailTime);
    m_refStatisticsManager->Register(m_refillGetBuffersTime);
    m_refStatisticsManager->Register(m_refillPostTime);
    m_refStatisticsManager->Register(m_eeSchedTime);

    m_refStatisticsManager->Register(m_postedWRQs);
    m_refStatisticsManager->Register(m_polledWRQs);
    m_refStatisticsManager->Register(m_queueEmptied);
    m_refStatisticsManager->Register(m_irbFull);
    m_refStatisticsManager->Register(m_receivedData);
    m_refStatisticsManager->Register(m_receivedFC);
    m_refStatisticsManager->Register(m_handlerNoProcess);
    m_refStatisticsManager->Register(m_refillInsufficientBuffers);

    m_refStatisticsManager->Register(m_bufferUtilization);
    m_refStatisticsManager->Register(m_fragmentedLastBuffer);
    m_refStatisticsManager->Register(m_fragmentedSGEs);

    m_refStatisticsManager->Register(m_throughputReceivedData);
    m_refStatisticsManager->Register(m_throughputReceivedFC);

    m_refStatisticsManager->Register(m_privateStats);
}

RecvDispatcher::~RecvDispatcher()
{
    delete m_ringBuffer;

    m_refStatisticsManager->Deregister(m_totalTime);

    m_refStatisticsManager->Deregister(m_pollTime);
    m_refStatisticsManager->Deregister(m_processRecvTotalTime);
    m_refStatisticsManager->Deregister(m_processRecvAvailTime);
    m_refStatisticsManager->Deregister(m_processRecvHandleTime);
    m_refStatisticsManager->Deregister(m_refillAvailTime);
    m_refStatisticsManager->Deregister(m_refillGetBuffersTime);
    m_refStatisticsManager->Deregister(m_refillPostTime);
    m_refStatisticsManager->Deregister(m_eeSchedTime);

    m_refStatisticsManager->Deregister(m_postedWRQs);
    m_refStatisticsManager->Deregister(m_polledWRQs);
    m_refStatisticsManager->Deregister(m_queueEmptied);
    m_refStatisticsManager->Deregister(m_irbFull);
    m_refStatisticsManager->Deregister(m_receivedData);
    m_refStatisticsManager->Deregister(m_receivedFC);
    m_refStatisticsManager->Deregister(m_handlerNoProcess);
    m_refStatisticsManager->Deregister(m_refillInsufficientBuffers);

    m_refStatisticsManager->Deregister(m_bufferUtilization);
    m_refStatisticsManager->Deregister(m_fragmentedLastBuffer);
    m_refStatisticsManager->Deregister(m_fragmentedSGEs);

    m_refStatisticsManager->Deregister(m_throughputReceivedData);
    m_refStatisticsManager->Deregister(m_throughputReceivedFC);

    m_refStatisticsManager->Deregister(m_privateStats);

    delete m_totalTime;

    delete m_pollTime;
    delete m_processRecvTotalTime;
    delete m_processRecvAvailTime;
    delete m_processRecvHandleTime;
    delete m_refillAvailTime;
    delete m_refillGetBuffersTime;
    delete m_refillPostTime;
    delete m_eeSchedTime;

    delete m_postedWRQs;
    delete m_polledWRQs;
    delete m_queueEmptied;
    delete m_irbFull;
    delete m_receivedData;
    delete m_receivedFC;
    delete m_handlerNoProcess;
    delete m_refillInsufficientBuffers;

    delete m_bufferUtilization;
    delete m_fragmentedLastBuffer;
    delete m_fragmentedSGEs;

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

    activity = __Poll();
    activity = __Refill() || activity;
    activity = __ProcessCompletions() || activity;
    activity = __DispatchReceived() || activity;

    IBNET_STATS(m_eeSchedTime->Start());

    return activity;
}

bool RecvDispatcher::__Poll()
{
    uint32_t ringBufferFree = m_ringBuffer->NumFreeEntries();

    // every work completion might hold up to max SGEs buffers filled
    ringBufferFree /= m_refConnectionManager->GetMaxSGEs();

    if (ringBufferFree > 0) {
        IBNET_STATS(m_pollTime->Start());

        // if we have more free space than completion queue size, cap
        if (ringBufferFree > m_refConnectionManager->GetIbSRQSize()) {
            ringBufferFree = m_refConnectionManager->GetIbSRQSize();
        }

        // poll in batches to reduce overhead and increase utilization
        int ret = ibv_poll_cq(m_refConnectionManager->GetIbSharedRCQ(), ringBufferFree, m_workComps);

        if (ret < 0) {
            __ThrowDetailedException<core::IbException>(ret, "Polling completion queue failed");
        }

        m_received = static_cast<uint32_t>(ret);
        IBNET_STATS(m_polledWRQs->Add(m_received));

        m_recvQueuePending -= m_received;

        // track if queue was emptied to get a indication of possible pipeline stalls (naks)
        if (m_recvQueuePending == 0) {
            IBNET_STATS(m_queueEmptied->Inc());
        }

        IBNET_STATS(m_pollTime->Stop());
    } else {
        // can't receive, no space in ring buffer. leave possible completions in CQ
        IBNET_STATS(m_irbFull->Inc());
        m_received = 0;
    }

    // space in comp list and work requests pending -> activity
    return m_received > 0;
}

bool RecvDispatcher::__Refill()
{
    if (m_recvQueuePending < m_refConnectionManager->GetIbSRQSize()) {
        IBNET_STATS(m_refillAvailTime->Start());

        // TODO refill in limited batches to ensure that at least a few buffers are as fast as possible
        // back in the RQ -> e.g. batches of 10?

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
                IBNET_STATS(m_refillInsufficientBuffers->Inc());
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

bool RecvDispatcher::__ProcessCompletions()
{
    if (m_received > 0) {
        IBNET_STATS(m_processRecvAvailTime->Start());

        // iterate work completions and check for errors
        for (uint32_t i = 0; i < m_received; i++) {
            if (m_workComps[i].status != IBV_WC_SUCCESS) {
                if (m_workComps[i].status) {
                    switch (m_workComps[i].status) {
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
                                core::WORK_COMPLETION_STATUS_CODE[m_workComps[i].status]);

                    }
                }
            }

            m_firstWc = false;

            // successful work completion, evaluate and add to IRB
            auto* immedData = (ImmediateData*) &m_workComps[i].imm_data;
            auto* recvWorkReq = (RecvWorkRequest*) m_workComps[i].wr_id;
            uint32_t dataRecvLen = m_workComps[i].byte_len;

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
                IncomingRingBuffer::RingBuffer::Entry* entry = m_ringBuffer->Back();

                entry->m_sourceNodeId = immedData->m_sourceNodeId;
                entry->m_fcData = immedData->m_flowControlData;
                entry->m_padding = 0xFF;
                entry->m_data = nullptr;
                entry->m_dataRaw = nullptr;
                entry->m_dataLength = 0;

                IBNET_STATS(m_receivedFC->Inc());

                m_ringBuffer->PushBack();
            } else {
                uint32_t dataRecvLenTmp = dataRecvLen;
                uint32_t sgesUsed = 0;
                uint32_t remainderDataOfLastBuffer = 0;

                // if multiple SGEs were provided on recv post, we have to figure out which buffer contains
                // received data using the total size
                for (uint32_t j = 0; j < recvWorkReq->m_sgls.m_numUsedElems; j++) {
                    IncomingRingBuffer::RingBuffer::Entry* entry = m_ringBuffer->Back();

                    entry->m_sourceNodeId = immedData->m_sourceNodeId;

                    // fc data with immediate data available, process once on first SGE
                    if (immedData->m_flowControlData) {
                        entry->m_fcData = immedData->m_flowControlData;
                        immedData->m_flowControlData = 0;
                        IBNET_STATS(m_receivedFC->Inc());
                    } else {
                        entry->m_fcData = 0;
                    }

                    entry->m_padding = 0xFF;
                    entry->m_data = recvWorkReq->m_sgls.m_refsMemReg[j];
                    entry->m_dataRaw = recvWorkReq->m_sgls.m_refsMemReg[j]->GetAddress();

                    uint32_t maxBufferSize = recvWorkReq->m_sgls.m_refsMemReg[j]->GetSizeBuffer();

                    // figure out how much data is in the scattered buffers
                    if (dataRecvLenTmp >= maxBufferSize) {
                        entry->m_dataLength = maxBufferSize;
                        dataRecvLenTmp -= maxBufferSize;
                    } else {
                        entry->m_dataLength = dataRecvLenTmp;
                        remainderDataOfLastBuffer = dataRecvLenTmp;
                        dataRecvLenTmp = 0;
                    }

                    m_ringBuffer->PushBack();

                    sgesUsed++;

                    if (dataRecvLenTmp == 0) {
                        break;
                    }
                }

                // track utilization degree on all buffers, don't count unused SGEs here
                IBNET_STATS(m_bufferUtilization->GetDenominator().Add(
                            sgesUsed * m_refRecvBufferPool->GetBufferSize()));
                IBNET_STATS(m_bufferUtilization->GetNumerator().Add(dataRecvLen));

                // track last buffer fragmentation degree
                if (remainderDataOfLastBuffer == 0) {
                    IBNET_STATS(m_fragmentedLastBuffer->GetNumerator().Add(remainderDataOfLastBuffer));
                } else {
                    IBNET_STATS(m_fragmentedLastBuffer->GetNumerator().Add(
                            m_refRecvBufferPool->GetBufferSize() - remainderDataOfLastBuffer));
                }

                IBNET_STATS(m_fragmentedLastBuffer->GetDenominator().Add(m_refRecvBufferPool->GetBufferSize()));

                // return unused SGEs
                uint32_t unused = recvWorkReq->m_sgls.m_numUsedElems - sgesUsed;

                // also track how many buffers of the SGE list were not used
                if (unused > 0) {
                    m_refRecvBufferPool->ReturnBuffers(recvWorkReq->m_sgls.m_refsMemReg + sgesUsed, unused);

                    IBNET_STATS(m_fragmentedSGEs->GetNumerator().Add(unused));
                }

                IBNET_STATS(m_fragmentedSGEs->GetDenominator().Add(recvWorkReq->m_sgls.m_numUsedElems));

                // return work request wrapper object
                m_recvWRPool->Push(recvWorkReq);

                IBNET_STATS(m_receivedData->Add(dataRecvLen));
            }
        }

        IBNET_STATS(m_processRecvAvailTime->Stop());
        return true;
    } else {
        return false;
    }
}

bool RecvDispatcher::__DispatchReceived()
{
    if (!m_ringBuffer->IsEmpty()) {
        IBNET_STATS(m_processRecvHandleTime->Start());

        // buffers are returned to recv buffer pool async
        uint32_t processed = m_refRecvHandler->Received(m_ringBuffer->GetRingBuffer());
//            uint32_t processed;
//            static int bla = 0;
//            if (bla > 100) {
//                for (uint32_t i = 0; i < m_recvPackage->m_count; i++) {
//                    m_refRecvBufferPool->ReturnBuffer(m_recvPackage->m_entries[i].m_data);
//                }
//                processed = m_recvPackage->m_count;
//            } else {
//                bla++;
//                processed = m_refRecvHandler->Received(m_recvPackage);
//            }

        m_ringBuffer->PopFront(processed);

        if (processed == 0) {
            // handler could not process anything (e.g. Java IBQ full)
            IBNET_STATS(m_handlerNoProcess->Inc());
        }

        IBNET_STATS(m_processRecvHandleTime->Stop());

        return true;
    } else {
        return false;
    }
}

}
}