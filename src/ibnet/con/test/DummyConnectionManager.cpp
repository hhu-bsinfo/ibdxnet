//
// Created by on 2/12/18.
//

#include "DummyConnectionManager.h"

#include "DummyConnection.h"

namespace ibnet {
namespace con {

DummyConnectionManager::DummyConnectionManager(
    NodeId ownNodeId, const NodeConf& nodeConf,
    uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
    core::IbDevice* refDevice, core::IbProtDom* refProtDom,
    ExchangeManager* refExchangeManager, JobManager* refJobManager,
    DiscoveryManager* refDiscoveryManager) :
    ConnectionManager("Dummy", ownNodeId, nodeConf, connectionCreationTimeoutMs,
        maxNumConnections, refDevice, refProtDom, refExchangeManager,
        refJobManager, refDiscoveryManager)
{

}

con::Connection* DummyConnectionManager::_CreateConnection(
    con::ConnectionId connectionId)
{
    return new DummyConnection(_GetOwnNodeId(), connectionId);
}

}
}