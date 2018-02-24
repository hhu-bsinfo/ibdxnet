/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_CON_DUMMYCONNECTIONMANAGER_H
#define IBNET_CON_DUMMYCONNECTIONMANAGER_H

#include "ibnet/con/ConnectionManager.h"

namespace ibnet {
namespace con {

/**
 * Dummy implementation of a connection manager to test and debug
 * connection management
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 12.02.2018
 */
class DummyConnectionManager : public ConnectionManager
{
public:
    /**
     * Constructor
     *
     * @param ownNodeId Node id of the current instance
     * @param nodeConf Node config to use
     * @param connectionCreationTimeoutMs Timeout for connection creation in ms
     * @param maxNumConnections Max number of connections to manage
     * @param refDevice Pointer to the IbDevice (managed by caller)
     * @param refProtDom Pointer to the IbProtDom (managed by caller)
     * @param refExchangeManager Pointer to exchange manager to use for
     *        managing connections (managed by caller)
     * @param refJobManager Pointer to job manager to use for managing
     *        connections (managed by caller)
     * @param refDiscoveryManager Pointer to discovery manager to use
     *        (managed by caller)
     */
    DummyConnectionManager(NodeId ownNodeId, const NodeConf& nodeConf,
        uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
        core::IbDevice* refDevice, core::IbProtDom* refProtDom,
        ExchangeManager* refExchangeManager, JobManager* refJobManager,
        DiscoveryManager* refDiscoveryManager);

    /**
     * Destructor
     */
    ~DummyConnectionManager() override = default;

protected:
    con::Connection* _CreateConnection(con::ConnectionId connectionId) override;
};

}
}

#endif //IBNET_CON_DUMMYCONNECTIONMANAGER_H
