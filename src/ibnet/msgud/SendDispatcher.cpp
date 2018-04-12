//
// Created by ruhland on 2/9/18.
//

#include "SendDispatcher.h"

#include "ibnet/sys/IllegalStateException.h"
#include "ibnet/sys/TimeoutException.h"

#include "ibnet/core/IbCommon.h"
#include "ibnet/core/IbQueueFullException.h"

#include "ibnet/con/DisconnectedException.h"

#include "Common.h"

namespace ibnet {
namespace msgud {

SendDispatcher::SendDispatcher(uint32_t recvBufferSize,
    uint8_t ackFrameSize,
    ConnectionManager* refConectionManager,
    stats::StatisticsManager* refStatisticsManager,
    SendHandler* refSendHandler) :
    ExecutionUnit("MsgUDSend"),
    m_recvBufferSize(recvBufferSize),
    m_ackFrameSize(ackFrameSize),
    m_refConnectionManager(refConectionManager),
    m_refStatisticsManager(refStatisticsManager),
    m_refSendHandler(refSendHandler),
    m_prevWorkPackageResults(static_cast<SendHandler::PrevWorkPackageResults*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(SendHandler::PrevWorkPackageResults)))),
    m_completionList(static_cast<SendHandler::CompletedWorkList*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            SendHandler::CompletedWorkList::Sizeof(
                refConectionManager->GetMaxNumConnections())))),
    m_completionsPending(0),
    m_sendQueuePending(),
    m_firstWc(true),
    m_ignoreFlushErrOnPendingCompletions(0),
    m_sgeLists(static_cast<ibv_sge*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(ibv_sge) * m_refConnectionManager->GetIbQPSize()))),
    m_sendWrs(static_cast<ibv_send_wr*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(ibv_send_wr) * m_refConnectionManager->GetIbQPSize()))),
    m_workComp(static_cast<ibv_wc*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
            sizeof(ibv_wc) * m_refConnectionManager->GetIbCQSize()))),
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
    m_sentData(new stats::Unit("SendDispatcher", "Data", stats::Unit::e_Base2)),
    m_sentFC(new stats::Unit("SendDispatcher", "FC", stats::Unit::e_Base10)),
    m_emptyNextWorkPackage(new stats::Unit("SendDispatcher", "EmptyNextWorkPackage")),
    m_nonEmptyNextWorkPackage(new stats::Unit("SendDispatcher", "NonEmptyNextWorkPackage")),
    m_sendDataFullBuffers(new stats::Unit("SendDispatcher", "DataFullBuffers")),
    m_sendDataNonFullBuffers(new stats::Unit("SendDispatcher", "DataNonFullBuffers")),
    m_sendBatches(new stats::Unit("SendDispatcher", "Batches")),
    m_emptyCompletionPolls(new stats::Unit("SendDispatcher", "EmptyCompletionPolls")),
    m_nonEmptyCompletionPolls(new stats::Unit("SendDispatcher", "NonEmptyCompletionPolls")),
    m_completionBatches(new stats::Unit("SendDispatcher", "CompletionBatches")),
    m_nextWorkPackageRatio(new stats::Ratio("SendDispatcher", "NextWorkPackageRatio",
        m_nonEmptyNextWorkPackage, m_emptyNextWorkPackage)),
    m_sendDataFullBuffersRatio(new stats::Ratio("SendDispatcher", "DataFullBuffersRatio",
        m_nonEmptyNextWorkPackage, m_emptyNextWorkPackage)),
    m_emptyCompletionPollsRatio(new stats::Ratio("SendDispatcher", "EmptyCompletionPollsRatio",
        m_nonEmptyCompletionPolls, m_emptyCompletionPolls)),
    m_throughputSentData(new stats::Throughput("SendDispatcher", "ThroughputData", m_sentData,
        m_totalTime)),
    m_throughputSentFC(new stats::Throughput("SendDispatcher", "ThroughputFC", m_sentFC,
        m_totalTime)),
    m_privateStats(new Stats(this))
{
    memset(m_prevWorkPackageResults, 0,
        sizeof(SendHandler::PrevWorkPackageResults));
    memset(m_completionList, 0, SendHandler::CompletedWorkList::Sizeof(
        refConectionManager->GetMaxNumConnections()));

    memset(m_sendQueuePending, 0,
        sizeof(uint16_t) * con::NODE_ID_MAX_NUM_NODES);

    memset(m_sgeLists, 0,
        sizeof(ibv_sge) * m_refConnectionManager->GetIbQPSize());
    memset(m_sendWrs, 0,
        sizeof(ibv_send_wr) * m_refConnectionManager->GetIbQPSize());
    memset(m_workComp, 0,
        sizeof(ibv_wc) * m_refConnectionManager->GetIbCQSize());
    
    m_refStatisticsManager->Register(m_totalTimeline);
    m_refStatisticsManager->Register(m_pollTimeline);
    m_refStatisticsManager->Register(m_sendTimeline);

    m_refStatisticsManager->Register(m_sentData);
    m_refStatisticsManager->Register(m_sentFC);

    m_refStatisticsManager->Register(m_emptyNextWorkPackage);
    m_refStatisticsManager->Register(m_nonEmptyNextWorkPackage);

    m_refStatisticsManager->Register(m_sendDataFullBuffers);
    m_refStatisticsManager->Register(m_sendDataNonFullBuffers);
    m_refStatisticsManager->Register(m_sendBatches);

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

    m_refStatisticsManager->Deregister(m_sentData);
    m_refStatisticsManager->Deregister(m_sentFC);

    m_refStatisticsManager->Deregister(m_emptyNextWorkPackage);
    m_refStatisticsManager->Deregister(m_nonEmptyNextWorkPackage);

    m_refStatisticsManager->Deregister(m_sendDataFullBuffers);
    m_refStatisticsManager->Deregister(m_sendDataNonFullBuffers);
    m_refStatisticsManager->Deregister(m_sendBatches);

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

    delete m_sentData;
    delete m_sentFC;

    delete m_emptyNextWorkPackage;
    delete m_nonEmptyNextWorkPackage;

    delete m_sendDataFullBuffers;
    delete m_sendDataNonFullBuffers;
    delete m_sendBatches;

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

    const SendHandler::NextWorkPackage* workPackage =
        m_refSendHandler->GetNextDataToSend(m_prevWorkPackageResults,
            m_completionList);

    if (workPackage == nullptr) {
        __ThrowDetailedException<sys::IllegalStateException>(
            "Work package null");
    }

    // reset previous states
    m_prevWorkPackageResults->Reset();
    m_completionList->Reset();

    Connection* connection = nullptr;
    bool ret;

    try {
        // nothing to send, poll completions
        if (workPackage->m_nodeId == con::NODE_ID_INVALID) {
            IBNET_STATS(m_emptyNextWorkPackage->Inc());

            IBNET_STATS(m_pollCompletionsTotalTime->Start());

            ret = __PollCompletions();

            IBNET_STATS(m_pollCompletionsTotalTime->Stop());
        } else {
            IBNET_STATS(m_nonEmptyNextWorkPackage->Inc());

            IBNET_STATS(m_getConnectionTime->Start());

            connection = (Connection*)
                m_refConnectionManager->GetConnection(workPackage->m_nodeId);

            IBNET_STATS(m_getConnectionTime->Stop());
            IBNET_STATS(m_sendDataTotalTime->Start());

            // send data
            ret = __SendData(connection, workPackage);

            IBNET_STATS(m_sentData->Add(
                m_prevWorkPackageResults->m_numBytesPosted));
            IBNET_STATS(m_sentFC->Add(m_prevWorkPackageResults->m_fcDataPosted));

            IBNET_STATS(m_sendDataTotalTime->Stop());

            IBNET_STATS(m_pollCompletionsTotalTime->Start());

            // after sending data, try polling for more completions
            ret = ret || __PollCompletions();

            IBNET_STATS(m_pollCompletionsTotalTime->Stop());

            m_refConnectionManager->ReturnConnection(connection);
        }
    } catch (sys::TimeoutException& e) {
        IBNET_LOG_WARN("TimeoutException: %s", e.what());

        IBNET_STATS(m_pollCompletionsTotalTime->Start());

        // timeout on initial connection creation
        // try polling work completions and return
        ret = __PollCompletions();

        IBNET_STATS(m_pollCompletionsTotalTime->Stop());
    } catch (con::DisconnectedException& e) {
        IBNET_LOG_WARN("DisconnectedException: %s", e.what());

        // if disconnect is detected during polling, connection is null
        if (connection) {
            m_refConnectionManager->ReturnConnection(connection);
        }

        m_refConnectionManager->CloseConnection(
            e.getNodeId(), true);

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
    if(m_completionsPending > 0) {
        IBNET_STATS(m_pollCompletionsActiveTime->Start());

        // poll in batches
        int ret = ibv_poll_cq(m_refConnectionManager->GetIbSendCQ(),
            m_refConnectionManager->GetIbCQSize(), m_workComp);
        
        if (ret < 0) {
            __ThrowDetailedException<core::IbException>(ret,
                "Polling completion queue failed");
        }

        if(ret > 0) {
            IBNET_STATS(m_nonEmptyCompletionPolls->Inc());
            IBNET_STATS(m_completionBatches->Add(static_cast<uint64_t>(ret)));

            for (uint32_t i = 0; i < static_cast<uint32_t>(ret); i++) {
                if (m_workComp[i].status != IBV_WC_SUCCESS) {
                    auto* ctx = (WorkRequestIdCtx*) &m_workComp[i].wr_id;

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
                                "Found failed work completion (%d), target "
                                    "node 0x%X, send bytes %d, fc data %d, "
                                    "status %s", i, ctx->m_targetNodeId,
                                ctx->m_sendSize, ctx->m_fcData,
                                core::WORK_COMPLETION_STATUS_CODE[
                                    m_workComp[i].status]);

                        case IBV_WC_RETRY_EXC_ERR:
                            if (m_firstWc) {
                                __ThrowDetailedException<core::IbException>(
                                    "First work completion of queue failed,"
                                        " it's very likely your connection "
                                        "attributes are wrong or the remote"
                                        " isn't in a state to respond");
                            } else {
                                throw con::DisconnectedException(
                                    ((ImmediateData*) &m_workComp[i].imm_data)->m_sourceNodeId);
                            }
                    }

                    // node failure/disconnect but still completions to poll
                    // from the cq. Continue decrementing until all failed
                    // completions are processed

                    m_sendQueuePending[ctx->m_targetNodeId]--;
                    m_completionsPending--;
                } else {
                    m_firstWc = false;

                    auto* ctx = (WorkRequestIdCtx*) &m_workComp[i].wr_id;

                    if (m_completionList->m_numBytesWritten[
                        ctx->m_targetNodeId] == 0 && m_completionList->
                        m_fcDataWritten[ctx->m_targetNodeId] == 0) {
                        m_completionList->m_nodeIds[
                            m_completionList->m_numNodes++] =
                            ctx->m_targetNodeId;
                    }

                    m_completionList->m_numBytesWritten[ctx->m_targetNodeId] +=
                        ctx->m_sendSize;
                    m_completionList->m_fcDataWritten[ctx->m_targetNodeId] +=
                        ctx->m_fcData;
                    m_sendQueuePending[ctx->m_targetNodeId]--;
                    m_completionsPending--;
                }

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

bool SendDispatcher::__SendData(Connection* connection,
        const SendHandler::NextWorkPackage* workPackage)
{
    IBNET_STATS(m_sendDataProcessingTime->Start());

    uint32_t totalBytesProcessed = 0;

    // get pointer to native send buffer (ORB)
    core::IbMemReg* refSendBuffer = connection->GetRefSendBuffer();

    con::NodeId nodeId = workPackage->m_nodeId;
    uint16_t queueSize = m_refConnectionManager->GetIbQPSize();

    uint8_t fcData = workPackage->m_flowControlData;
    uint32_t posBack = workPackage->m_posBackRel;
    uint32_t posFront = workPackage->m_posFrontRel;

    uint32_t totalBytesToProcess = 0;

    // wrap around
    if (posBack > posFront) {
        totalBytesToProcess = refSendBuffer->GetSize() - posBack + posFront;
    } else {
        totalBytesToProcess = posFront - posBack;
    }

    m_prevWorkPackageResults->m_nodeId = nodeId;

    uint16_t chunks = 0;
    uint16_t openConnections = m_refConnectionManager->getNumConnections();

    // slice area of send buffer into chunks fitting receive buffers
    // divide by number of connections, so that the send queue does not overflow
    while (m_sendQueuePending[nodeId] + chunks < (queueSize / openConnections) &&
           (posBack != posFront || fcData)) {
        uint32_t posEnd = posFront;

        // ring buffer wrap around detected: first, send everything
        // up to the end of the buffer (size)
        if (posBack > posFront) {
            // go to end of buffer, first
            posEnd = refSendBuffer->GetSize();
        }

        // end of buffer reached, wrap around to beginning of buffer
        // and continue
        if (posBack == posEnd) {
            posBack = 0;
            posEnd = posFront;
        }

        uint32_t length;
        bool zeroLength;

        // set this before moving posBack pointer
        m_sgeLists[chunks].addr =
            (uintptr_t) refSendBuffer->GetAddress() + posBack;

        // calculate length and move ring buffer pointers
        if (posBack + m_recvBufferSize <= posEnd) {
            // fits a full receive buffer
            length = m_recvBufferSize;
            zeroLength = false;

            posBack += length;
            totalBytesProcessed += length;

            IBNET_STATS(m_sendDataFullBuffers->Inc());
        } else {
            // smaller than a receive buffer
            length = posEnd - posBack;

            // zero length buffer, i.e. flow control data only
            if (length == 0) {
                zeroLength = true;
                length = 1;

                // sanity check
                if (!fcData) {
                    __ThrowDetailedException<sys::IllegalStateException>(
                        "Sending zero length data but no flow control");
                }
            } else {
                zeroLength = false;

                posBack += length;
                totalBytesProcessed += length;
            }

            IBNET_STATS(m_sendDataNonFullBuffers->Inc());
        }

        // don't send packages with size 0 which is a special value
        // and gets translated to 2^31 bytes length
        // use a flag to indicate 0 length payloads (see below)
        m_sgeLists[chunks].length = length;
        m_sgeLists[chunks].lkey = refSendBuffer->GetLKey();

        // context used on completion to identify completed work request
        auto* ctx = (WorkRequestIdCtx*) &m_sendWrs[chunks].wr_id;
        ctx->m_targetNodeId = nodeId;
        ctx->m_sendSize = zeroLength ? 0 : length;
        ctx->m_fcData = fcData;

        m_sendWrs[chunks].sg_list = &m_sgeLists[chunks];
        m_sendWrs[chunks].num_sge = 1;
        m_sendWrs[chunks].opcode = IBV_WR_SEND_WITH_IMM;
        m_sendWrs[chunks].send_flags = 0;

        auto* immedData = (ImmediateData*) &m_sendWrs[chunks].imm_data;
        immedData->m_sourceNodeId = connection->GetSourceNodeId();
        immedData->m_flowControlData = fcData;
        immedData->m_zeroLengthData = static_cast<uint8_t>(zeroLength ? 1 : 0);
        immedData->m_sequenceNumber = static_cast<uint8_t>(connection->GetSendSequenceNumber()->GetValue() %
                                                           m_ackFrameSize);
        immedData->m_endOfWorkPackage = 0;

        connection->GetSendSequenceNumber()->Inc();

        // fill in ud-data
        m_sendWrs[chunks].wr.ud.ah = connection->GetRefAddressHandle()->GetIbAh();
        m_sendWrs[chunks].wr.ud.remote_qpn = connection->GetRemotePhysicalQpId();
        m_sendWrs[chunks].wr.ud.remote_qkey = 0x22222222;

        // list is connected further down
        m_sendWrs[chunks].next = nullptr;

        if (fcData > 0) {
            m_prevWorkPackageResults->m_fcDataPosted = fcData;
            // send fc confirmation once and clear when done
            fcData = 0;
        }

        chunks++;
    }

    IBNET_STATS(m_sendDataProcessingTime->Stop());
    IBNET_STATS(m_sendDataPostingTime->Start());

    bool activity = false;

    if(chunks > 0) {
        activity = true;

        ((ImmediateData*) (&m_sendWrs[chunks - 1].imm_data))->m_endOfWorkPackage = 1;
        connection->GetSendSequenceNumber()->Reset();

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
        int ret = ibv_post_send(m_refConnectionManager->GetIbQP(), &m_sendWrs[0], &firstBadWr);

        if (ret != 0) {
            switch (ret) {
                case ENOMEM:
                    __ThrowDetailedException<core::IbQueueFullException>(
                        "Send queue full: %d", m_sendQueuePending[nodeId]);

                default:
                    __ThrowDetailedException<core::IbException>(ret,
                        "Posting work request to send to queue failed");
            }
        }

        IBNET_STATS(m_sendBatches->Add(chunks));

        m_sendQueuePending[nodeId] += chunks;
        // completion queue shared among all connections
        m_completionsPending += chunks;
    }

    if (fcData > 0) {
        m_prevWorkPackageResults->m_fcDataNotPosted = fcData;
    }

    m_prevWorkPackageResults->m_numBytesPosted = totalBytesProcessed;
    m_prevWorkPackageResults->m_numBytesNotPosted =
        totalBytesToProcess - totalBytesProcessed;

    IBNET_STATS(m_sendDataPostingTime->Stop());

    return activity;
}

}
}