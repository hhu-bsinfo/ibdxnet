//
// Created by nothaas on 1/30/18.
//

#include "Connection.h"

#include "ibnet/sys/Logger.hpp"
#include "ibnet/sys/IllegalStateException.h"
#include "ibnet/sys/Random.h"

#define DEFAULT_IB_PORT 1
#define IB_QOS_LEVEL 0

namespace ibnet {
namespace msgrc {

Connection::Connection(con::NodeId ownNodeId, con::ConnectionId connectionId,
        uint32_t sendBufferSize, uint16_t ibSQSize, ibv_srq* refIbSRQ,
        uint16_t ibSRQSize, ibv_cq* refIbSharedSCQ, uint16_t ibSharedSCQSize,
        ibv_cq* refIbSharedRCQ, uint16_t ibSharedRCQSize,
        core::IbProtDom* refProtDom) :
    con::Connection(ownNodeId, connectionId),
    m_sendBufferSize(sendBufferSize),
    m_refProtDom(refProtDom),
    m_sendBuffer(nullptr),
    m_ibQP(nullptr),
    m_ibPhysicalQPId(0xFFFFFFFF),
    m_ibPsn(sys::Random::Generate32()),
    m_remoteConnectionData(),
    m_ibSQSize(ibSQSize),
    m_refIbSRQ(refIbSRQ),
    m_ibSRQSize(ibSRQSize),
    m_refIbSharedSCQ(refIbSharedSCQ),
    m_ibSharedSCQSize(ibSharedSCQSize),
    m_refIbSharedRCQ(refIbSharedRCQ),
    m_ibSharedRCQSize(ibSharedRCQSize)
{
    IBNET_LOG_TRACE_FUNC;

    try {
        __CreateQP();
        __SetInitStateQP();
    } catch (...) {
        if (m_ibQP) {
            ibv_destroy_qp(m_ibQP);
        }

        throw;
    }

    IBNET_LOG_DEBUG("Allocate send buffer, size %d for connection id 0x%X",
        m_sendBufferSize, connectionId);

    m_sendBuffer = new core::IbMemReg(
        aligned_alloc(static_cast<size_t>(getpagesize()), m_sendBufferSize),
        m_sendBufferSize, true);

    m_refProtDom->Register(m_sendBuffer);

    IBNET_LOG_DEBUG("Created QP, qpNum 0x%X", m_ibPhysicalQPId);
}

Connection::~Connection()
{
    if (m_ibQP) {
        ibv_destroy_qp(m_ibQP);
    }

    if (m_sendBuffer) {
        m_refProtDom->Deregister(m_sendBuffer);
        delete m_sendBuffer;
    }
}

void Connection::CreateConnectionExchangeData(void* connectionDataBuffer,
        size_t connectionDataMaxSize, size_t* connectionDataActualSize)
{
    if (connectionDataMaxSize < sizeof(RemoteConnectionData)) {
        throw sys::IllegalStateException("Buffer too small");
    }

    auto* data = static_cast<RemoteConnectionData*>(connectionDataBuffer);

    data->m_physicalQPId = m_ibPhysicalQPId;
    data->m_psn = m_ibPsn;

    *connectionDataActualSize = sizeof(RemoteConnectionData);
}

void Connection::Connect(
        const con::RemoteConnectionHeader& remoteConnectionHeader,
        const void* remoteConnectionData, size_t remoteConnectionDataSize)
{
    if (remoteConnectionDataSize < sizeof(RemoteConnectionData)) {
        throw sys::IllegalStateException("Buffer too small");
    }

    auto* data = static_cast<const RemoteConnectionData*>(remoteConnectionData);

    m_remoteConnectionHeader = remoteConnectionHeader;
    m_remoteConnectionData = *data;

    // ready to recv must be set first
    __SetReadyToRecv();
    __SetReadyToSend();
}

void Connection::Close(bool force)
{
    IBNET_LOG_TRACE_FUNC;

    // TODO state change to not ready send and receive?
}

void Connection::__CreateQP()
{
    IBNET_LOG_TRACE_FUNC;

    ibv_qp_init_attr qp_init_attr = {};
    memset(&qp_init_attr, 0, sizeof(ibv_qp_init_attr));

    qp_init_attr.send_cq = m_refIbSharedSCQ;
    qp_init_attr.recv_cq = m_refIbSharedRCQ;
    qp_init_attr.qp_type = IBV_QPT_RC;

    qp_init_attr.srq = m_refIbSRQ;

    qp_init_attr.cap.max_send_wr = m_ibSQSize;
    qp_init_attr.cap.max_recv_wr = m_ibSRQSize;
    qp_init_attr.cap.max_send_sge = 1;
    qp_init_attr.cap.max_recv_sge = 1;
    qp_init_attr.cap.max_inline_data = 0;
    // only generate CQ elements on requested WQ elements
    qp_init_attr.sq_sig_all = 0;

    IBNET_LOG_TRACE("ibv_create_qp");
    m_ibQP = ibv_create_qp(m_refProtDom->GetIBProtDom(), &qp_init_attr);

    if (m_ibQP == nullptr) {
        throw core::IbException("Creating queue pair failed: %s",
            strerror(errno));
    }

    m_ibPhysicalQPId = m_ibQP->qp_num;
}

void Connection::__SetInitStateQP()
{
    IBNET_LOG_TRACE_FUNC;

    // init queue pair. the queue pair needs to be set to ready to
    // send and receive after this
    int result;
    ibv_qp_attr qp_attr = {};
    memset(&qp_attr, 0, sizeof(struct ibv_qp_attr));

    qp_attr.qp_state        = IBV_QPS_INIT;
    qp_attr.pkey_index      = 0;
    qp_attr.port_num        = DEFAULT_IB_PORT;
    qp_attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE;

    // modify queue pair attributes
    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_ibQP, &qp_attr,
        IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);

    if (result != 0) {
        throw core::IbException("Setting queue pair state to init failed: %s",
            strerror(result));
    }
}

void Connection::__SetReadyToSend()
{
    IBNET_LOG_TRACE_FUNC;

    ibv_qp_attr attr = {};
    int result = 0;
    memset(&attr, 0, sizeof(struct ibv_qp_attr));

    // change state to ready to send
    // (qp states needs to be ready to receive, first)

    attr.qp_state = IBV_QPS_RTS;
    // local ack timeout
    attr.timeout = 14;
    // retry count on no answer on primary path
    attr.retry_cnt = 7;
    // rnr=receiver not ready, 7 = infinite
    // (always wait until receiver state is ready to receive)
    attr.rnr_retry = 7;
    // packet sequence number of sender (current instance)
    attr.sq_psn = m_ibPsn;
    // nr of outstanding RDMA reads
    // & atomic ops on dest. qp
    attr.max_rd_atomic = 1;

    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_ibQP, &attr,
        IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY |
        IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);

    if (result != 0) {
        throw core::IbException("Setting queue pair to ready to send failed");
    }
}

void Connection::__SetReadyToRecv()
{
    IBNET_LOG_TRACE_FUNC;

    ibv_qp_attr attr = {};
    int result = 0;
    memset(&attr, 0, sizeof(struct ibv_qp_attr));

    attr.qp_state = IBV_QPS_RTR;
    attr.path_mtu = IBV_MTU_4096;
    // server qp_num
    attr.dest_qp_num = m_remoteConnectionData.m_physicalQPId;
    // packet sequence number of receiver
    attr.rq_psn = m_remoteConnectionData.m_psn;

    // num of responder resources for
    // incoming RDMA reads & atomic ops
    attr.max_dest_rd_atomic = 1;
    // minimum RNR NAK timer
    attr.min_rnr_timer = 12;
    // global routing header not used
    attr.ah_attr.is_global = 0;
    // LID of remote IB port
    attr.ah_attr.dlid = m_remoteConnectionHeader.m_lid;
    // QoS priority
    attr.ah_attr.sl = IB_QOS_LEVEL;
    // default port (for multiport NICs)
    attr.ah_attr.src_path_bits = 0;
    // IB port
    attr.ah_attr.port_num = DEFAULT_IB_PORT;

    // do the state change on the qp
    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_ibQP, &attr,
        IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
        IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);

    if (result != 0) {
        throw core::IbException(
            "Setting queue pair to ready to receive failed");
    }
}

}
}