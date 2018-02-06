//
// Created by nothaas on 1/30/18.
//

#include "ConnectionManager.h"

#include "ibnet/core/IbQueueFullException.h"

#include "Connection.h"

namespace ibnet {
namespace msgrc {

ConnectionManager::ConnectionManager(con::NodeId ownNodeId,
        const con::NodeConf& nodeConf, uint32_t connectionCreationTimeoutMs,
        uint32_t maxNumConnections, core::IbDevice* refDevice,
        core::IbProtDom* refProtDom, con::ExchangeManager* refExchangeManager,
        con::JobManager* refJobManager,
        con::DiscoveryManager* refDiscoveryManager, uint32_t sendBufferSize,
        uint16_t ibSQSize, uint16_t ibSRQSize, uint16_t ibSharedSCQSize,
        uint16_t ibSharedRCQSize) :
    con::ConnectionManager("MsgRC", ownNodeId, nodeConf,
        connectionCreationTimeoutMs, maxNumConnections, refDevice, refProtDom,
        refExchangeManager, refJobManager, refDiscoveryManager),
    m_sendBufferSize(sendBufferSize),
    m_ibSQSize(ibSQSize),
    m_ibSRQ(__CreateSRQ(ibSRQSize)),
    m_ibSRQSize(ibSRQSize),
    m_ibSharedSCQ(__CreateCQ(ibSharedSCQSize)),
    m_ibSharedSCQSize(ibSharedSCQSize),
    m_ibSharedRCQ(__CreateCQ(ibSharedRCQSize)),
    m_ibSharedRCQSize(ibSharedRCQSize),
    m_initialSRQFill(true)
{

}

ConnectionManager::~ConnectionManager()
{
    ibv_destroy_srq(m_ibSRQ);
    ibv_destroy_cq(m_ibSharedSCQ);
    ibv_destroy_cq(m_ibSharedRCQ);
}

con::Connection* ConnectionManager::_CreateConnection(
        con::ConnectionId connectionId)
{
    return new msgrc::Connection(_GetOwnNodeId(), connectionId,
        m_sendBufferSize, m_ibSQSize, m_ibSRQ, m_ibSRQSize, m_ibSharedSCQ,
        m_ibSharedSCQSize, m_ibSharedRCQ, m_ibSharedRCQSize, _GetRefProtDom());
}

ibv_srq* ConnectionManager::__CreateSRQ(uint16_t size)
{
    ibv_srq_init_attr attr = {};
    ibv_srq* srq;

    memset(&attr, 0, sizeof(attr));

    attr.attr.max_sge = 1;
    attr.attr.max_wr = size;

    IBNET_LOG_TRACE("ibv_create_srq, size %d", size);
    srq = ibv_create_srq(_GetRefProtDom()->GetIBProtDom(), &attr);

    if (srq == nullptr) {
        throw core::IbException("Creating shared receive queue failed: %s",
            strerror(errno));
    }
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
}

}
}