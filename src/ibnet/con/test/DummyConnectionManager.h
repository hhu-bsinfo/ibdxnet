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


//
// Created by on 2/12/18.
//
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
