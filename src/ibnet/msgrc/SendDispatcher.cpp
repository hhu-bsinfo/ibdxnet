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

#include "SendDispatcher.h"

#include "ibnet/sys/IllegalStateException.h"
#include "ibnet/sys/TimeoutException.h"

#include "ibnet/core/IbCommon.h"
#include "ibnet/core/IbQueueFullException.h"

#include "ibnet/con/DisconnectedException.h"

#include "Common.h"

namespace ibnet {
namespace msgrc {

SendDispatcher::SendDispatcher(uint32_t recvBufferSize,
        ConnectionManager* refConectionManager,
        stats::StatisticsManager* refStatisticsManager,
        SendHandler* refSendHandler) :
        ExecutionUnit("MsgRCSend"),
        m_recvBufferSize(recvBufferSize),
        m_refConnectionManager(refConectionManager),
        m_refStatisticsManager(refStatisticsManager),
        m_refSendHandler(refSendHandler),
        m_prevWorkPackageResults(static_cast<SendHandler::PrevWorkPackageResults*>(
                aligned_alloc(static_cast<size_t>(getpagesize()), sizeof(SendHandler::PrevWorkPackageResults)))),
        m_completionList(static_cast<SendHandler::CompletedWorkList*>(
                aligned_alloc(static_cast<size_t>(getpagesize()),
                        SendHandler::CompletedWorkList::Sizeof(refConectionManager->GetMaxNumConnections())))),
        m_completionsPending(0),
        m_sendQueuePending(),
        m_firstWc(true),
        m_ignoreFlushErrOnPendingCompletions(0),
        m_sgeLists(static_cast<ibv_sge*>(
                aligned_alloc(static_cast<size_t>(getpagesize()),
                        // *2 because max 2 SGEs per work request (split data on orb wrap around)
                        sizeof(ibv_sge) * m_refConnectionManager->GetIbSQSize() * 2))),
        m_sendWrs(static_cast<ibv_send_wr*>(
                aligned_alloc(static_cast<size_t>(getpagesize()),
                        sizeof(ibv_send_wr) * m_refConnectionManager->GetIbSQSize()))),
        m_workComp(static_cast<ibv_wc*>(
                aligned_alloc(static_cast<size_t>(getpagesize()),
                        sizeof(ibv_wc) * m_refConnectionManager->GetIbSharedSCQSize()))),
        m_workRequestCtxPool(new SendWorkRequestCtxPool(m_refConnectionManager->GetIbSharedSCQSize())),
        m_totalTime(new stats::Time("SendDispatcher", "Total")),
        m_getNextDataToSendTime(new stats::Time("SendDispatcher", "GetNextDataToSend")),
        m_pollCompletionsTotalTime(new stats::Time("SendDispatcher", "PollCompletionsTotal")),
        m_pollCompletionsActiveTime(new stats::Time("SendDispatcher", "PollCompletions")),
        m_getConnectionTime(new stats::Time("SendDispatcher", "GetConnection")),
        m_sendDataTotalTime(new stats::Time("SendDispatcher", "SendDataTotal")),
        m_sendDataProcessingTime(new stats::Time("SendDispatcher", "SendDataProcessing")),
        m_sendDataPostingTime(new stats::Time("SendDispatcher", "SendDataPosting")),
        m_eeScheduleTime(new stats::Time("SendDispatcher", "EESchedule")),
        m_totalTimeline(new stats::TimelineFragmented("SendDispatcher", "Total", m_totalTime, {m_getNextDataToSendTime,
                m_pollCompletionsTotalTime, m_getConnectionTime, m_sendDataTotalTime, m_eeScheduleTime})),
        m_pollTimeline(new stats::TimelineFragmented("SendDispatcher", "Poll", m_pollCompletionsTotalTime,
                {m_pollCompletionsActiveTime})),
        m_sendTimeline(new stats::TimelineFragmented("SendDispatcher", "Send", m_sendDataTotalTime,
                {m_sendDataProcessingTime, m_sendDataPostingTime})),
        m_postedWRQs(new stats::Unit("SendDispatcher", "WRQsPosted", stats::Unit::e_Base10)),
        m_postedDataChunk(new stats::Unit("SendDispatcher", "PostedDataChunk", stats::Unit::e_Base2)),
        m_postedDataRemainderChunk(new stats::Unit("SendDispatcher", "PostedDataRemainderChunk", stats::Unit::e_Base2)),
        m_sentData(new stats::Unit("SendDispatcher", "Data", stats::Unit::e_Base2)),
        m_sentFC(new stats::Unit("SendDispatcher", "FC", stats::Unit::e_Base10)),
        m_emptyNextWorkPackage(new stats::Unit("SendDispatcher", "EmptyNextWorkPackage")),
        m_nonEmptyNextWorkPackage(new stats::Unit("SendDispatcher", "NonEmptyNextWorkPackage")),
        m_sendDataFullBuffers(new stats::Unit("SendDispatcher", "DataFullBuffers")),
        m_sendDataNonFullBuffers(new stats::Unit("SendDispatcher", "DataNonFullBuffers")),
        m_sendBatches(new stats::Distribution("SendDispatcher", "Batches", 10)),
        m_sendPostedLens(new stats::Distribution("SendDispatcher", "SendPostLens", 5)),
        m_sendQueueFull(new stats::Unit("SendDispatcher", "QueueFull")),
        m_emptyCompletionPolls(new stats::Unit("SendDispatcher", "EmptyCompletionPolls")),
        m_nonEmptyCompletionPolls(new stats::Unit("SendDispatcher", "NonEmptyCompletionPolls")),
        m_completionBatches(new stats::Unit("SendDispatcher", "CompletionBatches")),
        m_nextWorkPackageRatio(new stats::Ratio("SendDispatcher", "NextWorkPackageRatio",
                m_nonEmptyNextWorkPackage, m_emptyNextWorkPackage)),
        m_sendDataFullBuffersRatio(new stats::Ratio("SendDispatcher", "DataFullBuffersRatio",
                m_nonEmptyNextWorkPackage, m_emptyNextWorkPackage)),
        m_emptyCompletionPollsRatio(new stats::Ratio("SendDispatcher", "EmptyCompletionPollsRatio",
                m_nonEmptyCompletionPolls, m_emptyCompletionPolls)),
        m_throughputSentData(new stats::Throughput("SendDispatcher", "ThroughputData", m_sentData, m_totalTime)),
        m_throughputSentFC(new stats::Throughput("SendDispatcher", "ThroughputFC", m_sentFC, m_totalTime)),
        m_privateStats(new Stats(this))
{
    memset(m_prevWorkPackageResults, 0, sizeof(SendHandler::PrevWorkPackageResults));
    memset(m_completionList, 0, SendHandler::CompletedWorkList::Sizeof(refConectionManager->GetMaxNumConnections()));

    // set correct initial state
    m_prevWorkPackageResults->Reset();
    m_completionList->Reset();

    memset(m_sendQueuePending, 0, sizeof(uint16_t) * con::NODE_ID_MAX_NUM_NODES);

    memset(m_sgeLists, 0, sizeof(ibv_sge) * m_refConnectionManager->GetIbSQSize() * 2);
    memset(m_sendWrs, 0, sizeof(ibv_send_wr) * m_refConnectionManager->GetIbSQSize());
    memset(m_workComp, 0, sizeof(ibv_wc) * m_refConnectionManager->GetIbSharedSCQSize());

    m_refStatisticsManager->Register(m_totalTimeline);
    m_refStatisticsManager->Register(m_pollTimeline);
    m_refStatisticsManager->Register(m_sendTimeline);

    m_refStatisticsManager->Register(m_postedWRQs);
    m_refStatisticsManager->Register(m_postedDataChunk);
    m_refStatisticsManager->Register(m_postedDataRemainderChunk);

    m_refStatisticsManager->Register(m_sentData);
    m_refStatisticsManager->Register(m_sentFC);

    m_refStatisticsManager->Register(m_emptyNextWorkPackage);
    m_refStatisticsManager->Register(m_nonEmptyNextWorkPackage);

    m_refStatisticsManager->Register(m_sendDataFullBuffers);
    m_refStatisticsManager->Register(m_sendDataNonFullBuffers);
    m_refStatisticsManager->Register(m_sendBatches);
    m_refStatisticsManager->Register(m_sendPostedLens);
    m_refStatisticsManager->Register(m_sendQueueFull);

    m_refStatisticsManager->Register(m_emptyCompletionPolls);
    m_refStatisticsManager->Register(m_nonEmptyCompletionPolls);
    m_refStatisticsManager->Register(m_completionBatches);

    m_refStatisticsManager->Register(m_nextWorkPackageRatio);
    m_refStatisticsManager->Register(m_sendDataFullBuffersRatio);
    m_refStatisticsManager->Register(m_emptyCompletionPollsRatio);

    m_refStatisticsManager->Register(m_throughputSentData);
    m_refStatisticsManager->Register(m_throughputSentFC);

    m_refStatisticsManager->Register(m_privateStats);
}

SendDispatcher::~SendDispatcher()
{
    m_refStatisticsManager->Deregister(m_totalTimeline);
    m_refStatisticsManager->Deregister(m_pollTimeline);
    m_refStatisticsManager->Deregister(m_sendTimeline);

    m_refStatisticsManager->Deregister(m_postedWRQs);
    m_refStatisticsManager->Deregister(m_postedDataChunk);
    m_refStatisticsManager->Deregister(m_postedDataRemainderChunk);

    m_refStatisticsManager->Deregister(m_sentData);
    m_refStatisticsManager->Deregister(m_sentFC);

    m_refStatisticsManager->Deregister(m_emptyNextWorkPackage);
    m_refStatisticsManager->Deregister(m_nonEmptyNextWorkPackage);

    m_refStatisticsManager->Deregister(m_sendDataFullBuffers);
    m_refStatisticsManager->Deregister(m_sendDataNonFullBuffers);
    m_refStatisticsManager->Deregister(m_sendBatches);
    m_refStatisticsManager->Deregister(m_sendPostedLens);
    m_refStatisticsManager->Deregister(m_sendQueueFull);

    m_refStatisticsManager->Deregister(m_emptyCompletionPolls);
    m_refStatisticsManager->Deregister(m_nonEmptyCompletionPolls);
    m_refStatisticsManager->Deregister(m_completionBatches);

    m_refStatisticsManager->Deregister(m_nextWorkPackageRatio);
    m_refStatisticsManager->Deregister(m_sendDataFullBuffersRatio);
    m_refStatisticsManager->Deregister(m_emptyCompletionPollsRatio);

    m_refStatisticsManager->Deregister(m_throughputSentData);
    m_refStatisticsManager->Deregister(m_throughputSentFC);

    m_refStatisticsManager->Deregister(m_privateStats);

    free(m_prevWorkPackageResults);
    free(m_completionList);

    free(m_sgeLists);
    free(m_sendWrs);
    free(m_workComp);

    delete (m_workRequestCtxPool);

    delete m_totalTime;
    delete m_pollTimeline;
    delete m_sendTimeline;

    delete m_getNextDataToSendTime;
    delete m_pollCompletionsTotalTime;
    delete m_pollCompletionsActiveTime;
    delete m_getConnectionTime;
    delete m_sendDataTotalTime;
    delete m_sendDataProcessingTime;
    delete m_sendDataPostingTime;
    delete m_eeScheduleTime;

    delete m_totalTimeline;

    delete m_postedWRQs;
    delete m_postedDataChunk;
    delete m_postedDataRemainderChunk;

    delete m_sentData;
    delete m_sentFC;

    delete m_emptyNextWorkPackage;
    delete m_nonEmptyNextWorkPackage;

    delete m_sendDataFullBuffers;
    delete m_sendDataNonFullBuffers;
    delete m_sendBatches;
    delete m_sendPostedLens;
    delete m_sendQueueFull;

    delete m_emptyCompletionPolls;
    delete m_nonEmptyCompletionPolls;
    delete m_completionBatches;

    delete m_nextWorkPackageRatio;
    delete m_sendDataFullBuffersRatio;
    delete m_emptyCompletionPollsRatio;

    delete m_throughputSentData;
    delete m_throughputSentFC;

    delete m_privateStats;
}

bool SendDispatcher::Dispatch()
{
    IBNET_STATS(m_eeScheduleTime->Stop());

    if (m_totalTime->GetCounter() == 0) {
        IBNET_STATS(m_totalTime->Start());
    } else {
        IBNET_STATS(m_totalTime->Stop());
        IBNET_STATS(m_totalTime->Start());
    }

    IBNET_STATS(m_getNextDataToSendTime->Start());

    const SendHandler::NextWorkPackage* workPackage = m_refSendHandler->GetNextDataToSend(m_prevWorkPackageResults,
            m_completionList);

    IBNET_STATS(m_getNextDataToSendTime->Stop());

    if (workPackage == nullptr) {
        __ThrowDetailedException<sys::IllegalStateException>("Work package null");
    }

    // reset previous states
    m_prevWorkPackageResults->Reset();
    m_completionList->Reset();

    Connection* connection = nullptr;
    bool ret = false;

    try {
        // nothing to send, poll completions
        if (workPackage->m_nodeId == con::NODE_ID_INVALID) {
            IBNET_STATS(m_emptyNextWorkPackage->Inc());
        } else {
            IBNET_STATS(m_nonEmptyNextWorkPackage->Inc());

            IBNET_STATS(m_getConnectionTime->Start());

            connection = (Connection*) m_refConnectionManager->GetConnection(workPackage->m_nodeId);

            IBNET_STATS(m_getConnectionTime->Stop());
            IBNET_STATS(m_sendDataTotalTime->Start());

            // send data
            ret = __SendData(connection, workPackage);

            IBNET_STATS(m_sentData->Add(m_prevWorkPackageResults->m_numBytesPosted));
            IBNET_STATS(m_sentFC->Add(m_prevWorkPackageResults->m_fcDataPosted));

            IBNET_STATS(m_sendDataTotalTime->Stop());

            m_refConnectionManager->ReturnConnection(connection);
        }

        IBNET_STATS(m_pollCompletionsTotalTime->Start());

        ret = __PollCompletions() || ret;

        IBNET_STATS(m_pollCompletionsTotalTime->Stop());
    } catch (sys::TimeoutException& e) {
        IBNET_LOG_WARN("Timeout: %s", e.what());

        IBNET_STATS(m_pollCompletionsTotalTime->Start());

        // timeout on initial connection creation
        // try polling work completions and return
        ret = __PollCompletions();

        IBNET_STATS(m_pollCompletionsTotalTime->Stop());
    } catch (con::DisconnectedException& e) {
        IBNET_LOG_WARN("Disconnected: %s", e.what());

        m_refConnectionManager->ReturnConnection(connection);
        m_refConnectionManager->CloseConnection(connection->GetRemoteNodeId(), true);

        // reset due to failure
        m_prevWorkPackageResults->Reset();

        m_ignoreFlushErrOnPendingCompletions += m_completionsPending;

        IBNET_STATS(m_eeScheduleTime->Start());

        ret = true;
    }

    IBNET_STATS(m_eeScheduleTime->Start());

    return ret;
}

bool SendDispatcher::__PollCompletions()
{
    // anything to poll from the shared completion queue
    if (m_completionsPending > 0) {
        IBNET_STATS(m_pollCompletionsActiveTime->Start());

        // poll in batches
        int ret = ibv_poll_cq(m_refConnectionManager->GetIbSharedSCQ(), m_refConnectionManager->GetIbSharedSCQSize(),
                m_workComp);

        if (ret < 0) {
            __ThrowDetailedException<core::IbException>(ret, "Polling completion queue failed");
        }

        if (ret > 0) {
            IBNET_STATS(m_nonEmptyCompletionPolls->Inc());
            IBNET_STATS(m_completionBatches->Add(static_cast<uint64_t>(ret)));

            for (uint32_t i = 0; i < static_cast<uint32_t>(ret); i++) {
                auto ctx = (SendWorkRequestCtx*) m_workComp[i].wr_id;

                if (m_workComp[i].status != IBV_WC_SUCCESS) {
                    switch (m_workComp[i].status) {
                        case IBV_WC_WR_FLUSH_ERR:
                            if (m_ignoreFlushErrOnPendingCompletions != 0) {
                                // some node disconnected/failed and we have
                                // to ignore any errors for a bit
                                break;
                            }

                            // fall through to default error case

                        default:
                            __ThrowDetailedException<core::IbException>(
                                    "Found failed work completion (%d), ctx: %s, status %s", i,
                                    *ctx, core::WORK_COMPLETION_STATUS_CODE[m_workComp[i].status]);

                        case IBV_WC_RETRY_EXC_ERR:
                            if (m_firstWc) {
                                __ThrowDetailedException<core::IbException>(
                                        "First work completion of queue failed,"
                                                " it's very likely your connection "
                                                "attributes are wrong or the remote"
                                                " isn't in a state to respond");
                            } else {
                                uint16_t nodeId = ctx->m_targetNodeId;
                                m_workRequestCtxPool->Push(ctx);
                                throw con::DisconnectedException(nodeId);
                            }
                    }

                    // node failure/disconnect but still completions to poll
                    // from the cq. Continue decrementing until all failed
                    // completions are processed

                    m_sendQueuePending[ctx->m_targetNodeId]--;
                    m_completionsPending--;
                } else {
                    m_firstWc = false;

                    if (m_completionList->m_numBytesWritten[ctx->m_targetNodeId] == 0 &&
                            m_completionList->m_fcDataWritten[ctx->m_targetNodeId] == 0) {
                        m_completionList->m_nodeIds[m_completionList->m_numNodes++] = ctx->m_targetNodeId;
                    }

                    m_completionList->m_numBytesWritten[ctx->m_targetNodeId] += ctx->m_sendSize;
                    m_completionList->m_fcDataWritten[ctx->m_targetNodeId] += ctx->m_fcData;
                    m_sendQueuePending[ctx->m_targetNodeId]--;
                    m_completionsPending--;
                }

                m_workRequestCtxPool->Push(ctx);

                if (m_ignoreFlushErrOnPendingCompletions > 0) {
                    m_ignoreFlushErrOnPendingCompletions--;
                }
            }
        } else {
            IBNET_STATS(m_emptyCompletionPolls->Inc());
        }

        IBNET_STATS(m_pollCompletionsActiveTime->Stop());
    }

    return m_completionsPending > 0;
}

bool SendDispatcher::__SendData(Connection* connection, const SendHandler::NextWorkPackage* workPackage)
{
    // TODO remove m_sendPostedLens

    IBNET_STATS(m_sendDataProcessingTime->Start());
    uint32_t chunks = __SendDataPrepareWorkRequests(connection, workPackage);
    IBNET_STATS(m_sendDataProcessingTime->Stop());

    // no data available
    if (chunks > 0) {
        __SendDataPostWorkRequests(connection, chunks);
        return true;
    } else {
        IBNET_STATS(m_sendQueueFull->Inc());
        return false;
    }
}

uint32_t SendDispatcher::__SendDataPrepareWorkRequests(Connection* connection,
        const SendHandler::NextWorkPackage* workPackage)
{
    const uint32_t maxRecvBufferSize = m_recvBufferSize * m_refConnectionManager->GetMaxSGEs();

    // get pointer to native send buffer (ORB)
    const core::IbMemReg* refSendBuffer = connection->GetRefSendBuffer();

    const con::NodeId nodeId = workPackage->m_nodeId;
    const uint32_t posFront = workPackage->m_posFrontRel;

    // states for processing
    uint8_t fcData = workPackage->m_flowControlData;
    uint32_t posBack = workPackage->m_posBackRel;
    uint32_t chunksPos = 0;
    uint32_t sgeListPos = 0;
    uint32_t totalBytesProcessed = 0;
    uint8_t totalFcDataProcessed = 0;

    uint32_t totalBytesToProcess = 0;

    // determine max amount of data available to send
    // wrap around
    if (posBack > posFront) {
        totalBytesToProcess = refSendBuffer->GetSizeBuffer() - posBack + posFront;
    } else {
        totalBytesToProcess = posFront - posBack;
    }

    while (m_sendQueuePending[nodeId] + chunksPos < m_refConnectionManager->GetIbSQSize() &&
            (posBack != posFront || fcData)) {
        // fc data only branch
        if (posBack == posFront && fcData) {
            // context used on completion to identify completed work request
            SendWorkRequestCtx* ctx = m_workRequestCtxPool->Pop();
            m_sendWrs[chunksPos].wr_id = (uint64_t) ctx;
            ctx->m_targetNodeId = nodeId;
            ctx->m_fcData = fcData;
            ctx->m_sendSize = 0;
            ctx->m_posFront = 0;
            ctx->m_posBack = 0;
            ctx->m_posEnd = 0;
            ctx->m_debug = 0;

            m_sendWrs[chunksPos].sg_list = nullptr;
            m_sendWrs[chunksPos].num_sge = 0;

            auto immedData = (ImmediateData*) &m_sendWrs[chunksPos].imm_data;
            immedData->m_sourceNodeId = connection->GetSourceNodeId();
            immedData->m_flowControlData = fcData;

            chunksPos++;

            totalFcDataProcessed += fcData;
            fcData = 0;

            break;
        } else {
            uint8_t debug = 0;

            // we got data (and probably FC data as well)

            // determine current end for work request -> consider wrap around
            uint32_t posEnd;
            bool wrapAround;

            // determine area to process for current work request, can be of max size maxRecvBuffer
            if (posBack > posFront) {
                if (posFront == 0) {
                    // edge case, front at start (0) and no wrap around for data, area enclosed: back to end of buffer
                    if (posBack + maxRecvBufferSize > refSendBuffer->GetSizeBuffer()) {
                        posEnd = refSendBuffer->GetSizeBuffer();
                        debug = 1;
                    } else {
                        posEnd = posBack + maxRecvBufferSize;
                        debug = 2;
                    }

                    wrapAround = false;
                } else if (posBack + maxRecvBufferSize <= refSendBuffer->GetSizeBuffer()) {
                    // no wrap around, yet
                    posEnd = posBack + maxRecvBufferSize;
                    wrapAround = false;
                    debug = 3;
                } else {
                    // wrap around
                    uint32_t areaAtEndOfBufferSize = refSendBuffer->GetSizeBuffer() - posBack;

                    // go to end of buffer first and determine how much we have to take from start of buffer
                    if (posFront + areaAtEndOfBufferSize > maxRecvBufferSize) {
                        posEnd = maxRecvBufferSize - areaAtEndOfBufferSize;
                        debug = 4;
                    } else {
                        posEnd = posFront;
                        debug = 5;
                    }

                    wrapAround = true;
                }
            } else {
                if (posBack + maxRecvBufferSize > posFront) {
                    posEnd = posFront;
                    debug = 6;
                } else {
                    posEnd = posBack + maxRecvBufferSize;
                    debug = 7;
                }

                wrapAround = false;
            }

            // no wrap around + fills maxRecvBuffer or partially -> 1 SGE
            if (!wrapAround) {
                uint32_t length = posEnd - posBack;

                // context used on completion to identify completed work request
                SendWorkRequestCtx* ctx = m_workRequestCtxPool->Pop();
                m_sendWrs[chunksPos].wr_id = (uint64_t) ctx;
                ctx->m_targetNodeId = nodeId;
                ctx->m_fcData = fcData;
                ctx->m_sendSize = length;
                ctx->m_posFront = posFront;
                ctx->m_posBack = posBack;
                ctx->m_posEnd = posEnd;
                ctx->m_debug = static_cast<uint8_t>(debug + 10);

                m_sgeLists[sgeListPos].addr = (uintptr_t) refSendBuffer->GetAddress() + posBack;
                m_sgeLists[sgeListPos].length = length;
                m_sgeLists[sgeListPos].lkey = refSendBuffer->GetLKey();

                m_sendWrs[chunksPos].sg_list = &m_sgeLists[sgeListPos];
                m_sendWrs[chunksPos].num_sge = 1;

                // sanity check
                if (length > maxRecvBufferSize) {
                    throw sys::IllegalStateException("Total length %d > max recv buffer size %d", length,
                            maxRecvBufferSize);
                }

                auto immedData = (ImmediateData*) &m_sendWrs[chunksPos].imm_data;
                immedData->m_sourceNodeId = connection->GetSourceNodeId();
                immedData->m_flowControlData = fcData;

                sgeListPos++;

                totalBytesProcessed += length;
            } else {
                // wrap around with 2 SGEs
                m_sendWrs[chunksPos].sg_list = &m_sgeLists[sgeListPos];
                m_sendWrs[chunksPos].num_sge = 2;

                uint32_t totalLength = 0;

                // 1: posBack to ORB size/end
                m_sgeLists[sgeListPos].addr = (uintptr_t) refSendBuffer->GetAddress() + posBack;
                m_sgeLists[sgeListPos].length = refSendBuffer->GetSizeBuffer() - posBack;
                m_sgeLists[sgeListPos].lkey = refSendBuffer->GetLKey();
                totalLength += m_sgeLists[sgeListPos].length;
                sgeListPos++;

                // 2: 0 to what fills the receive buffer
                m_sgeLists[sgeListPos].addr = (uintptr_t) refSendBuffer->GetAddress() + 0;
                // max - previous sge size
                m_sgeLists[sgeListPos].length = posEnd;//maxRecvBufferSize - totalLength;
//                // but limit to posEnd
//                if (m_sgeLists[sgeListPos].length > posEnd) {
//                    m_sgeLists[sgeListPos].length = posEnd;
//                }

                m_sgeLists[sgeListPos].lkey = refSendBuffer->GetLKey();
                totalLength += m_sgeLists[sgeListPos].length;
                sgeListPos++;

                // sanity check
                if (totalLength > maxRecvBufferSize) {
                    throw sys::IllegalStateException("Total length %d > max recv buffer size %d", totalLength,
                            maxRecvBufferSize);
                }

                // context used on completion to identify completed work request
                SendWorkRequestCtx* ctx = m_workRequestCtxPool->Pop();
                m_sendWrs[chunksPos].wr_id = (uint64_t) ctx;
                ctx->m_targetNodeId = nodeId;
                ctx->m_fcData = fcData;
                ctx->m_sendSize = totalLength;
                ctx->m_posFront = posFront;
                ctx->m_posBack = posBack;
                ctx->m_posEnd = posEnd;
                ctx->m_debug = static_cast<uint8_t>(debug + 20);

                auto immedData = (ImmediateData*) &m_sendWrs[chunksPos].imm_data;
                immedData->m_sourceNodeId = connection->GetSourceNodeId();
                immedData->m_flowControlData = fcData;

                totalBytesProcessed += totalLength;
            }

            m_sendWrs[chunksPos].opcode = IBV_WR_SEND_WITH_IMM;
            m_sendWrs[chunksPos].send_flags = 0;
            // list is connected further down
            m_sendWrs[chunksPos].next = nullptr;

            chunksPos++;

            // move orb pointer
            posBack = posEnd;

            // handle wrap around exactly on buffer size
            if (posBack == refSendBuffer->GetSizeBuffer()) {
                posBack = 0;
            }

            // include fcData once
            if (fcData > 0) {
                totalFcDataProcessed += fcData;
                fcData = 0;
            }

            // check if end of area to process is reached
            if (posBack == posFront) {
                break;
            }
        }
    }

    // prepare work package results
    m_prevWorkPackageResults->m_nodeId = nodeId;

    m_prevWorkPackageResults->m_fcDataNotPosted = fcData;
    m_prevWorkPackageResults->m_fcDataPosted = totalFcDataProcessed;

    m_prevWorkPackageResults->m_numBytesPosted = totalBytesProcessed;
    m_prevWorkPackageResults->m_numBytesNotPosted = totalBytesToProcess - totalBytesProcessed;

    IBNET_STATS(m_postedDataChunk->Add(totalBytesProcessed));
    // only record here to get an idea of much data was possible to be posted if there is actually space
    // in the queue
    IBNET_STATS(m_postedDataRemainderChunk->Add(totalBytesToProcess - totalBytesProcessed));

    return chunksPos;
}

void SendDispatcher::__SendDataPostWorkRequests(Connection* connection, uint32_t chunks)
{
    IBNET_STATS(m_sendDataPostingTime->Start());

    // connect all work requests
    for (uint16_t i = 0; i < chunks - 1; i++) {
        m_sendWrs[i].next = &m_sendWrs[i + 1];
    }

    // note: some tests have shown that it seems like ack'ing every nth
    // work request is a bad idea and increases overall latency. polling
    // in batches already deals with generating completions
    // for every work request quite well
    for (uint16_t i = 0; i < chunks; i++) {
        // Just signal all work requests, seems like this
        // doesn't make any difference performance wise
        m_sendWrs[i].send_flags = IBV_SEND_SIGNALED;
    }

    ibv_send_wr* firstBadWr;

    // batch post
    int ret = ibv_post_send(connection->GetQP(), &m_sendWrs[0], &firstBadWr);

    if (ret != 0) {
        switch (ret) {
            case ENOMEM:
                __ThrowDetailedException<core::IbQueueFullException>(
                        "Send queue full: %d", m_sendQueuePending[connection->GetRemoteNodeId()]);

            default:
                __ThrowDetailedException<core::IbException>(ret,
                        "Posting work request to send to queue failed");
        }
    }

    IBNET_STATS(m_postedWRQs->Add(chunks));

    m_sendQueuePending[connection->GetRemoteNodeId()] += chunks;
    // completion queue shared among all connections
    m_completionsPending += chunks;

    IBNET_STATS(m_sendBatches->GetUnit((float) chunks / m_refConnectionManager->GetIbSQSize()).Inc());

    IBNET_STATS(m_sendDataPostingTime->Stop());
}

}
}