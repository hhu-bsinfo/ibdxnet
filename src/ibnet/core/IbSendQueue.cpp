#include "IbSendQueue.h"

#include "ibnet/sys/Logger.hpp"

#include "IbQueueFullException.h"
#include "IbQueuePair.h"

namespace ibnet {
namespace core {

IbSendQueue::IbSendQueue(std::shared_ptr<IbDevice>& device,
        IbQueuePair& parentQp, uint16_t queueSize) :
    m_parentQp(parentQp),
    m_queueSize(queueSize),
    m_isClosed(false),
    // create non shared/private comp queue, match size of max work requests
    m_compQueue(std::make_unique<IbCompQueue>(device, queueSize))
{

}

IbSendQueue::~IbSendQueue()
{
    // delete private queue
    m_compQueue.reset();
}

void IbSendQueue::Open(void)
{
    IBNET_LOG_TRACE_FUNC;

    struct ibv_qp_attr attr;
    int result = 0;

    // change state to ready to send
    // (qp states needs to be ready to receive, first)
    memset(&attr, 0, sizeof(struct ibv_qp_attr));

    // ready to send state
    attr.qp_state = IBV_QPS_RTS;
    // local ack timeout
    attr.timeout = 14;
    // retry count on no answer on primary path
    attr.retry_cnt = 7;
    // rnr=receiver not ready, 7 = infinite
    // (always wait until receiver state is ready to receive)
    attr.rnr_retry = 7;
    // packet sequence number
    attr.sq_psn = 0;
    attr.rq_psn = 0;
    // nr of outstanding RDMA reads
    // & atomic ops on dest. qp
    attr.max_rd_atomic = 1;

    /* do the state change on the qp*/
    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_parentQp.GetIbQp(), &attr,
        IBV_QP_STATE            |
        IBV_QP_TIMEOUT          |
        IBV_QP_RETRY_CNT        |
        IBV_QP_RNR_RETRY        |
        IBV_QP_SQ_PSN           |
        IBV_QP_MAX_QP_RD_ATOMIC);

    if (result != 0) {
        IBNET_LOG_ERROR(
            "Setting queue pair to ready to send to connection failed");
        throw IbException("Setting queue pair to ready to send failed");
    }
}

void IbSendQueue::Close(bool force)
{
    if (!force) {
        // wait until outstanding completions are finished
        while (m_compQueue->GetCurrentOutstandingCompletions() > 0) {
            std::this_thread::yield();
        }
    }

    m_isClosed = true;
}

void IbSendQueue::Send(const IbMemReg* memReg, uint32_t offset, uint32_t size,
        uint64_t workReqId)
{
    struct ibv_sge sge_list;
    struct ibv_send_wr wr;
    // first failed work request
    struct ibv_send_wr *bad_wr;

    if (m_isClosed) {
        throw IbQueueClosedException();
    }

    // hook memory with message contents to send
    sge_list.addr      		= (uintptr_t) memReg->GetAddress() + offset;
    if (size == -1) {
        sge_list.length = (uint32_t) memReg->GetSize();
    } else {
        sge_list.length = (uint32_t) size;
    }
    sge_list.lkey      		= memReg->GetLKey();

    // work request for send operation
    wr.wr_id       			= workReqId;
    wr.sg_list     			= &sge_list;
    wr.num_sge     			= 1;
    wr.opcode      			= IBV_WR_SEND;
    // set completion notification
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.next        			= NULL;

    int ret = ibv_post_send(m_parentQp.GetIbQp(), &wr, &bad_wr);
    if (ret != 0) {
        switch (ret) {
            case ENOMEM:
                throw IbQueueFullException(
                    "Send queue full, compQueue outstanding: " +
                        std::to_string(
                            m_compQueue->GetCurrentOutstandingCompletions()));
            default:
                throw IbException(
                    "Posting work request to send to queue failed (" +
                    std::string(strerror(ret)) + ", mem: " +
                    memReg->ToString());
        }
    }

    m_compQueue->AddOutstandingCompletion();
}

uint32_t IbSendQueue::PollCompletion(bool blocking)
{
    if (m_isClosed) {
        throw IbQueueClosedException();
    }

    return m_compQueue->PollForCompletion(blocking);
}

uint32_t IbSendQueue::Flush(void)
{
    if (m_isClosed) {
        throw IbQueueClosedException();
    }

    return m_compQueue->Flush();
}

}
}