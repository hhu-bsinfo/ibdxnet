//
// Created by nothaas on 1/30/18.
//

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
    m_prevWorkPackageResults(new SendHandler::PrevWorkPackageResults()),
    m_completionList(static_cast<SendHandler::CompletedWorkList*>(
        malloc(SendHandler::CompletedWorkList::Sizeof(
        refConectionManager->GetMaxNumConnections())))),
    m_completionsPending(0),
    m_sendQueuePending(),
    m_firstWc(true),
    m_sgeLists(static_cast<ibv_sge*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(ibv_sge) * m_refConnectionManager->GetIbSQSize()))),
    m_sendWrs(static_cast<ibv_send_wr*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(ibv_send_wr) * m_refConnectionManager->GetIbSQSize()))),
    m_workComp(static_cast<ibv_wc*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(ibv_wc) * m_refConnectionManager->GetIbSharedSCQSize()))),
    m_totalTime(new stats::Time("SendTotalTime")),
    m_pollCompletions(new stats::Time("PollCompletionsTime")),
    m_sendData(new stats::Time("SendDataTime")),
    m_sentData(new stats::Unit("SentData", stats::Unit::e_Base2)),
    m_sentFC(new stats::Unit("SentFC", stats::Unit::e_Base10)),
    m_emptyNextWorkPackage(new stats::Unit("SendEmptyNextWorkPackage")),
    m_nonEmptyNextWorkPackage(new stats::Unit("SendNonEmptyNextWorkPackage")),
    m_sendDataFullBuffers(new stats::Unit("SendDataFullBuffers")),
    m_sendDataNonFullBuffers(new stats::Unit("SendDataNonFullBuffers")),
    m_sendBatches(new stats::Unit("SendBatches")),
    m_emptyCompletionPolls(new stats::Unit("SendEmptyCompletionPolls")),
    m_nonEmptyCompletionPolls(new stats::Unit("SendNonEmptyCompletionPolls")),
    m_completionBatches(new stats::Unit("SendCompletionBatches")),
    m_nextWorkPackageRatio(new stats::Ratio("SendNextWorkPackageRatio",
         m_nonEmptyNextWorkPackage, m_emptyNextWorkPackage)),
    m_sendDataFullBuffersRatio(new stats::Ratio("SendDataFullBuffersRatio",
        m_nonEmptyNextWorkPackage, m_emptyNextWorkPackage)),
    m_emptyCompletionPollsRatio(new stats::Ratio(
        "SendEmptyCompletionPollsRatio", m_nonEmptyCompletionPolls,
        m_emptyCompletionPolls)),
    m_throughputSentData(new stats::Throughput("SentThroughputData", m_sentData,
        m_totalTime)),
    m_throughputSentFC(new stats::Throughput("SentThroughputFC", m_sentFC,
        m_totalTime))
{
    memset(m_prevWorkPackageResults, 0,
        sizeof(SendHandler::PrevWorkPackageResults));
    memset(m_completionList, 0, SendHandler::CompletedWorkList::Sizeof(
        refConectionManager->GetMaxNumConnections()));

    // set correct initial state
    m_prevWorkPackageResults->Reset();
    m_completionList->Reset();

    memset(m_sendQueuePending, 0,
        sizeof(uint16_t) * con::NODE_ID_MAX_NUM_NODES);

    memset(m_sgeLists, 0,
        sizeof(ibv_sge) * m_refConnectionManager->GetIbSQSize());
    memset(m_sendWrs, 0,
        sizeof(ibv_send_wr) * m_refConnectionManager->GetIbSQSize());
    memset(m_workComp, 0,
        sizeof(ibv_wc) * m_refConnectionManager->GetIbSharedSCQSize());

    m_refStatisticsManager->Register(m_totalTime);
    m_refStatisticsManager->Register(m_pollCompletions);
    m_refStatisticsManager->Register(m_sendData);

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
}

SendDispatcher::~SendDispatcher()
{
    m_refStatisticsManager->Deregister(m_totalTime);
    m_refStatisticsManager->Deregister(m_pollCompletions);
    m_refStatisticsManager->Deregister(m_sendData);

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

    delete m_prevWorkPackageResults;
    free(m_completionList);

    free(m_sgeLists);
    free(m_workComp);
    free(m_workComp);

    delete m_totalTime;
    delete m_pollCompletions;
    delete m_sendData;

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
}

bool SendDispatcher::Dispatch()
{
    if (m_totalTime->GetCounter() == 0) {
        m_totalTime->Start();
    } else {
        m_totalTime->Stop();
        m_totalTime->Start();
    }

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

    try {
        // nothing to send, poll completions
        if (workPackage->m_nodeId == con::NODE_ID_INVALID) {
            m_emptyNextWorkPackage->Inc();
            return __PollCompletions();
        }

        m_nonEmptyNextWorkPackage->Inc();

        connection = (Connection*)
            m_refConnectionManager->GetConnection(workPackage->m_nodeId);

        // send data
        __SendData(connection, workPackage);

        m_sentData->Add(m_prevWorkPackageResults->m_numBytesPosted);
        m_sentFC->Add(m_prevWorkPackageResults->m_fcDataPosted);

        // TODO trigger park start not correct, yet

        // after sending data, try polling for more completions
        return __PollCompletions();
    } catch (sys::TimeoutException& e) {
        IBNET_LOG_WARN("TimeoutException: %s", e.what());

        // timeout on initial connection creation
        // try polling work completions and return
        return __PollCompletions();
    } catch (con::DisconnectedException& e) {
        IBNET_LOG_WARN("DisconnectedException: %s", e.what());

        m_refConnectionManager->ReturnConnection(connection);
        m_refConnectionManager->CloseConnection(
            connection->GetRemoteNodeId(), true);

        // reset due to failure
        m_prevWorkPackageResults->Reset();

        return true;
    }
}

bool SendDispatcher::__PollCompletions()
{
    // anything to poll from the shared completion queue
    if (m_completionsPending > 0) {
        m_pollCompletions->Start();

        // poll in batches
        int ret = ibv_poll_cq(m_refConnectionManager->GetIbSharedSCQ(),
            m_refConnectionManager->GetIbSharedSCQSize(), m_workComp);

        if (ret < 0) {
            __ThrowDetailedException<core::IbException>(ret,
                "Polling completion queue failed");
        }

        if (ret > 0) {
            m_nonEmptyCompletionPolls->Inc();
            m_completionBatches->Add(static_cast<uint64_t>(ret));

            for (uint32_t i = 0; i < static_cast<uint32_t>(ret); i++) {
                if (m_workComp[i].status != IBV_WC_SUCCESS) {
                    if (m_workComp[i].status) {
                        switch (m_workComp[i].status) {
                            // a previous work request failed and put the queue into error
                            // state
                            //case IBV_WC_WR_FLUSH_ERR:
                            //    throw IbException("Work completion of recv queue failed, "
                            //        "flush err");

                            case IBV_WC_RETRY_EXC_ERR:
                                if (m_firstWc) {
                                    __ThrowDetailedException<core::IbException>(
                                        "First work completion of queue failed,"
                                        " it's very likely your connection "
                                        "attributes are wrong or the remote"
                                        " isn't in a state to respond");
                                } else {
                                    throw con::DisconnectedException();
                                }

                            default:
                                auto* ctx =
                                    (WorkRequestIdCtx*) &m_workComp[i].wr_id;

                                __ThrowDetailedException<core::IbException>(
                                    "Found failed work completion (%d), target "
                                    "node 0x%X, send bytes %d, fc data %d, "
                                    "status %s", i, ctx->m_targetNodeId,
                                    ctx->m_sendSize, ctx->m_fcData,
                                    core::WORK_COMPLETION_STATUS_CODE[
                                        m_workComp[i].status]);
                        }

                        // TODO case if a node got disconnected but there are
                        // still completions to poll from the cq
                        // have to handle them here, further decrement
                        // m_completionsPending--;
                        // and reset m_sendQueueFillState[nodeId] = 0;
                        // on disconnect detection
                    }
                } else {
                    m_firstWc = false;

                    auto* ctx =
                        (WorkRequestIdCtx*) &m_workComp[i].wr_id;

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
            }
        } else {
            m_emptyCompletionPolls->Inc();
        }

        m_pollCompletions->Stop();
    }

    return m_completionsPending > 0;
}

void SendDispatcher::__SendData(Connection* connection,
        const SendHandler::NextWorkPackage* workPackage)
{
    m_sendData->Start();

    uint32_t totalBytesProcessed = 0;

    // get pointer to native send buffer (ORB)
    core::IbMemReg* refSendBuffer = connection->GetRefSendBuffer();

    con::NodeId nodeId = workPackage->m_nodeId;
    uint16_t queueSize = m_refConnectionManager->GetIbSQSize();

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

    // slice area of send buffer into chunks fitting receive buffers
    while (m_sendQueuePending[nodeId] + chunks < queueSize &&
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

       // edge case: wrap around exactly on buffer size and no new data after
       // wrap around
        if (posBack == posEnd && !fcData) {
            break;
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

            m_sendDataFullBuffers->Inc();
        } else {
            // smaller than a receive buffer
            length = posEnd - posBack;

            // zero length buffer, i.e. flow control data only
            if (length == 0) {
                zeroLength = true;
                length = 1;
            } else {
                zeroLength = false;

                posBack += length;
                totalBytesProcessed += length;
            }

            m_sendDataNonFullBuffers->Inc();
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
        // list is connected further down
        m_sendWrs[chunks].next = nullptr;

        auto* immedData = (ImmediateData*) &m_sendWrs[chunks].imm_data;
        immedData->m_sourceNodeId = connection->GetSourceNodeId();
        immedData->m_flowControlData = fcData;
        immedData->m_zeroLengthData = static_cast<uint8_t>(zeroLength ? 1 : 0);

        if (fcData > 0) {
            m_prevWorkPackageResults->m_fcDataPosted = fcData;
            // send fc confirmation once and clear when done
            fcData = 0;
        }

        chunks++;
    }

    if (chunks > 0) {
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
                        "Send queue full: %d", m_sendQueuePending[nodeId]);

                default:
                    __ThrowDetailedException<core::IbException>(ret,
                        "Posting work request to send to queue failed");
            }
        }

        m_sendBatches->Add(chunks);

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

    m_sendData->Stop();
}

}
}