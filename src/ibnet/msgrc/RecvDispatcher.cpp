//
// Created by nothaas on 1/30/18.
//

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
        refConnectionManager->GetIbSRQSize())))),
    m_recvQueuePending(0),
    m_workComps(static_cast<ibv_wc*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(ibv_wc) * refConnectionManager->GetIbSRQSize()))),
    m_firstWc(true),
    m_memRegRefillBuffer(static_cast<core::IbMemReg**>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(core::IbMemReg*) * refConnectionManager->GetIbSRQSize()))),
    m_sgeList(static_cast<ibv_sge*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(ibv_sge) * refConnectionManager->GetIbSRQSize()))),
    m_recvWrList(static_cast<ibv_recv_wr*>(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        sizeof(ibv_recv_wr) * refConnectionManager->GetIbSRQSize()))),
    m_totalTime(new stats::Time("RecvTotalTime")),
    m_receivedData(new stats::Unit("RecvData", stats::Unit::e_Base2)),
    m_receivedFC(new stats::Unit("RecvFC", stats::Unit::e_Base10)),
    m_throughputReceivedData(new stats::Throughput("RecvThroughputData",
        m_receivedData, m_totalTime)),
    m_throughputReceivedFC(new stats::Throughput("RecvThroughputFC",
        m_receivedFC, m_totalTime))
{
    memset(m_recvPackage, 0, RecvHandler::ReceivedPackage::Sizeof(
        refConnectionManager->GetIbSRQSize()));
    memset(m_workComps, 0,
        sizeof(ibv_wc) * refConnectionManager->GetIbSRQSize());
    memset(m_memRegRefillBuffer, 0,
        sizeof(core::IbMemReg*) * refConnectionManager->GetIbSRQSize());
    memset(m_sgeList, 0,
        sizeof(core::IbMemReg*) * refConnectionManager->GetIbSRQSize());
    memset(m_recvWrList, 0,
        sizeof(ibv_sge) * refConnectionManager->GetIbSRQSize());

    m_refStatisticsManager->Register(m_totalTime);
    m_refStatisticsManager->Register(m_receivedData);
    m_refStatisticsManager->Register(m_receivedFC);
    m_refStatisticsManager->Register(m_throughputReceivedData);
    m_refStatisticsManager->Register(m_throughputReceivedFC);
}

RecvDispatcher::~RecvDispatcher()
{
    free(m_recvPackage);

    m_refStatisticsManager->Deregister(m_totalTime);
    m_refStatisticsManager->Deregister(m_receivedData);
    m_refStatisticsManager->Deregister(m_receivedFC);
    m_refStatisticsManager->Deregister(m_throughputReceivedData);
    m_refStatisticsManager->Deregister(m_throughputReceivedFC);

    free(m_workComps);
    free(m_memRegRefillBuffer);
    free(m_sgeList);
    free(m_recvWrList);

    delete m_totalTime;
    delete m_receivedData;
    delete m_receivedFC;
    delete m_throughputReceivedData;
    delete m_throughputReceivedFC;
}

bool RecvDispatcher::Dispatch()
{
    if (m_totalTime->GetCounter() == 0) {
        IBNET_STATS(m_totalTime->Start());
    } else {
        IBNET_STATS(m_totalTime->Stop());
        IBNET_STATS(m_totalTime->Start());
    }

    uint32_t receivedCount = __Poll();
    __ProcessReceived(receivedCount);
    __Refill();

    return receivedCount > 0;
}

uint32_t RecvDispatcher::__Poll()
{
    // poll in batches to reduce overhead and increase utilization
    int ret = ibv_poll_cq(m_refConnectionManager->GetIbSharedRCQ(),
        m_refConnectionManager->GetIbSRQSize(), m_workComps);

    if (ret < 0) {
        __ThrowDetailedException<core::IbException>(
            ret, "Polling completion queue failed");
    }

    auto receivedCount = static_cast<uint32_t>(ret);

    // polling successful, iterate work completions and check for errors
    for (uint32_t i = 0; i < receivedCount; i++) {
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
    if (receivedCount > 0) {
        // create batch for handler

        // batch process all completions
        for (uint32_t i = 0; i < receivedCount; i++) {
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

        // buffers are returned to recv buffer pool async
        m_refRecvHandler->Received(m_recvPackage);
        m_recvPackage->m_count = 0;
    }
}

void RecvDispatcher::__Refill()
{
    if (m_recvQueuePending < m_refConnectionManager->GetIbSRQSize()) {
        uint32_t count =
            m_refConnectionManager->GetIbSRQSize() - m_recvQueuePending;

        // TODO test and fix batch getting from pool
//        uint32_t numBufs = m_refRecvBufferPool->GetBuffers(m_memRegRefillBuffer,
//            count);
        uint32_t numBufs = count;
        for (uint32_t i = 0; i < numBufs; i++) {
            m_memRegRefillBuffer[i] = m_refRecvBufferPool->GetBuffer();
        }

        // TODO track the batch sizes pulled from the completion queue and added back to the recv queue

        // first failed work request
        ibv_recv_wr* bad_wr;

        for (uint32_t i = 0; i < numBufs; i++) {
            // hook memory to write the received data to
            m_sgeList[i].addr =
                (uintptr_t) m_memRegRefillBuffer[i]->GetAddress();
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

        int ret = ibv_post_srq_recv(m_refConnectionManager->GetIbSRQ(),
            &m_recvWrList[0], &bad_wr);

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
    }
}

}
}