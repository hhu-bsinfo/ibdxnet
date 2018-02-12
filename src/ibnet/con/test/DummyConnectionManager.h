//
// Created by on 2/12/18.
//

#ifndef IBNET_CON_DUMMYCONNECTIONMANAGER_H
#define IBNET_CON_DUMMYCONNECTIONMANAGER_H

#include "ibnet/con/ConnectionManager.h"

namespace ibnet {
namespace con {

class DummyConnectionManager : public ConnectionManager
{
public:
    DummyConnectionManager(
            NodeId ownNodeId, const NodeConf& nodeConf,
            uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
            core::IbDevice* refDevice, core::IbProtDom* refProtDom,
            ExchangeManager* refExchangeManager, JobManager* refJobManager,
            DiscoveryManager* refDiscoveryManager);

    ~DummyConnectionManager() override = default;

protected:
    con::Connection* _CreateConnection(con::ConnectionId connectionId) override;
};

}
}

#endif //IBNET_CON_DUMMYCONNECTIONMANAGER_H
