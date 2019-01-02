//
// Created by ruhland on 2/8/18.
//

#include "ConnectionManager.h"

#include "Connection.h"

#define DEFAULT_IB_PORT 1

namespace ibnet {
namespace msgud {

ConnectionManager::ConnectionManager(
        con::NodeId ownNodeId, const con::NodeConf& nodeConf,
        uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
        core::IbDevice* refDevice, core::IbProtDom* refProtDom,
        con::ExchangeManager* refExchangeManager, con::JobManager* refJobManager,
        con::DiscoveryManager* refDiscoveryManager, uint32_t sendBufferSize,
        uint16_t ibQPSize, uint16_t ibCQSize) :
    con::ConnectionManager("MsgUD", ownNodeId, nodeConf,
        connectionCreationTimeoutMs, maxNumConnections, refDevice, refProtDom,
        refExchangeManager, refJobManager, refDiscoveryManager),
    m_refProtDom(refProtDom),
    m_sendBufferSize(sendBufferSize),
    m_ibQPSize(ibQPSize),
    m_ibCQSize(ibCQSize)
{
    m_ibSendCQ = __CreateCQ(m_ibCQSize);
    m_ibRecvCQ = __CreateCQ(m_ibCQSize);
    m_ibQP = __CreateQP(m_ibQPSize);

    __SetInitStateQP();
    __SetReadyToRecvQP();
    __SetReadyToSendQP();

    m_physicalQPId = m_ibQP->qp_num;

    m_numConnections = static_cast<uint16_t>(nodeConf.GetEntries().size());
}

ConnectionManager::~ConnectionManager()
{
    ibv_destroy_cq(m_ibSendCQ);
    ibv_destroy_cq(m_ibRecvCQ);
    ibv_destroy_qp(m_ibQP);
}

con::Connection* ConnectionManager::_CreateConnection(
        con::ConnectionId connectionId)
{
    return new msgud::Connection(_GetOwnNodeId(), connectionId,
        m_sendBufferSize, m_physicalQPId, _GetRefProtDom());
}

ibv_qp* ConnectionManager::__CreateQP(uint16_t size)
{
    IBNET_LOG_TRACE_FUNC;

    ibv_qp_init_attr attr{};
    memset(&attr, 0, sizeof(ibv_qp_init_attr));

    attr.send_cq = m_ibSendCQ;
    attr.recv_cq = m_ibRecvCQ;
    attr.qp_type = IBV_QPT_UD;

    attr.cap.max_send_wr = size;
    attr.cap.max_recv_wr = size;
    attr.cap.max_send_sge = 1;
    attr.cap.max_recv_sge = 1;
    attr.cap.max_inline_data = 0;
    // only generate CQ elements on requested WQ elements
    attr.sq_sig_all = 0;

    IBNET_LOG_TRACE("ibv_create_qp");
    m_ibQP = ibv_create_qp(m_refProtDom->GetIBProtDom(), &attr);

    if (m_ibQP == nullptr) {
        throw core::IbException("Creating queue pair failed: %s",
            strerror(errno));
    }

    return m_ibQP;
}

ibv_cq* ConnectionManager::__CreateCQ(uint16_t size)
{
    ibv_cq* cq;

    IBNET_LOG_TRACE("ibv_create_cq, size %d", size);
    cq = ibv_create_cq(_GetRefDevice()->GetIBCtx(), size, nullptr, nullptr, 0);

    if (cq == nullptr) {
        throw core::IbException("Creating completion queue failed: %s",
            strerror(errno));
    }

    return cq;
}

void ConnectionManager::__SetInitStateQP() {
    IBNET_LOG_TRACE_FUNC;

    int result;
    ibv_qp_attr attr{};
    memset(&attr, 0, sizeof(struct ibv_qp_attr));

    attr.qp_state        = IBV_QPS_INIT;
    attr.pkey_index      = 0;
    attr.port_num        = DEFAULT_IB_PORT;
    attr.qkey            = 0x22222222;

    // modify queue pair attributes
    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_ibQP, &attr,
        IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_QKEY);

    if (result != 0) {
        throw core::IbException("Setting queue pair state to init failed: %s",
            strerror(result));
    }
}

void ConnectionManager::__SetReadyToRecvQP() {
    IBNET_LOG_TRACE_FUNC;

    int result;
    ibv_qp_attr attr{};
    memset(&attr, 0, sizeof(struct ibv_qp_attr));

    attr.qp_state        = IBV_QPS_RTR;

    // modify queue pair attributes
    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_ibQP, &attr, IBV_QP_STATE);

    if (result != 0) {
        throw core::IbException("Setting queue pair state to ready to receive failed: %s",
            strerror(result));
    }
}

void ConnectionManager::__SetReadyToSendQP() {
    IBNET_LOG_TRACE_FUNC;

    int result;
    ibv_qp_attr attr{};
    memset(&attr, 0, sizeof(struct ibv_qp_attr));

    attr.qp_state        = IBV_QPS_RTS;

    // modify queue pair attributes
    IBNET_LOG_TRACE("ibv_modify_qp");
    result = ibv_modify_qp(m_ibQP, &attr, IBV_QP_STATE | IBV_QP_SQ_PSN);

    if (result != 0) {
        throw core::IbException("Setting queue pair state to ready to send failed: %s",
            strerror(result));
    }
}

}
}