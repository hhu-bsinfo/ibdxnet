//
// Created by nothaas on 1/30/18.
//

#ifndef IBNET_MSGRC_CONNECTIONMANAGER_H
#define IBNET_MSGRC_CONNECTIONMANAGER_H

#include "ibnet/con/ConnectionManager.h"

#include "ibnet/dx/RecvBufferPool.h"

namespace ibnet {
namespace msgrc {

class ConnectionManager : public con::ConnectionManager
{
public:
    ConnectionManager(con::NodeId ownNodeId, const con::NodeConf& nodeConf,
        uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
        core::IbDevice* refDevice, core::IbProtDom* refProtDom,
        con::ExchangeManager* refExchangeManager,
        con::JobManager* refJobManager,
        con::DiscoveryManager* refDiscoveryManager, uint32_t sendBufferSize,
        uint16_t ibSQSize, uint16_t ibSRQSize, uint16_t ibSharedSCQSize,
        uint16_t ibSharedRCQSize);

    ~ConnectionManager() override;

    uint16_t GetIbSQSize() const {
        return m_ibSQSize;
    }

    ibv_srq* GetIbSRQ() const {
        return m_ibSRQ;
    }

    uint16_t GetIbSRQSize() const {
        return m_ibSRQSize;
    }

    ibv_cq* GetIbSharedSCQ() const {
        return m_ibSharedSCQ;
    }

    uint16_t GetIbSharedSCQSize() const {
        return m_ibSharedSCQSize;
    }

    ibv_cq* GetIbSharedRCQ() const {
        return m_ibSharedRCQ;
    }

    uint16_t GetIbSharedRCQSize() const {
        return m_ibSharedRCQSize;
    }

protected:
    con::Connection* _CreateConnection(con::ConnectionId connectionId) override;

private:
    const uint32_t m_sendBufferSize;

    const uint16_t m_ibSQSize;

    ibv_srq* m_ibSRQ;
    const uint16_t m_ibSRQSize;

    ibv_cq* m_ibSharedSCQ;
    const uint16_t m_ibSharedSCQSize;

    ibv_cq* m_ibSharedRCQ;
    const uint16_t m_ibSharedRCQSize;

private:
    std::atomic<bool> m_initialSRQFill;

private:
    ibv_srq* __CreateSRQ(uint16_t size);
    ibv_cq* __CreateCQ(uint16_t size);
};

}
}

#endif //IBNET_MSGRC_CONNECTIONMANAGER_H
