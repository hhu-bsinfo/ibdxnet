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

#ifndef IBNET_MSGUD_CONNECTIONMANAGER_H
#define IBNET_MSGUD_CONNECTIONMANAGER_H

#include "ibnet/con/ConnectionManager.h"

namespace ibnet {
namespace msgud {

/**
 * Connection manager for unreliable messaging using a UD queue pair.
 *
 * @author Fabian Ruhland, fabian.ruhland@hhu.de, 08.02.2018
 */
class ConnectionManager : public con::ConnectionManager
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
     * @param sendBufferSize Size of the send (ring) buffer in bytes
     * @param ibQPSize Size of the queue pair
     * @param ibCQSize Size of the completion queues
     */
    ConnectionManager(con::NodeId ownNodeId, const con::NodeConf& nodeConf,
        uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
        core::IbDevice* refDevice, core::IbProtDom* refProtDom,
        con::ExchangeManager* refExchangeManager, con::JobManager* refJobManager,
        con::DiscoveryManager* refDiscoveryManager, uint32_t sendBufferSize,
        uint16_t ibQPSize, uint16_t ibCQSize);

    /**
     * Destructor
     */
    ~ConnectionManager() override;

    /**
     * Get the size of the queue pair
     */
    uint16_t GetIbQPSize() const {
        return m_ibQPSize;
    }

    /**
     * Get the size of the completion queues
     */
    uint16_t GetIbCQSize() const {
        return m_ibCQSize;
    }

    /**
     * Get the queue pair
     */
    ibv_qp* GetIbQP() {
        return m_ibQP;
    }

    /**
     * Get the receive completion queue
     */
    ibv_cq* GetIbRecvCQ() {
        return m_ibRecvCQ;
    }

    /**
     * Get the send completion queue
     */
    ibv_cq* GetIbSendCQ() {
        return m_ibSendCQ;
    }

    /**
     * Get the amount of connections, specified by the node configuration
     */
    uint16_t getNumConnections() {
        return m_numConnections;
    }

protected:
    con::Connection* _CreateConnection(con::ConnectionId connectionId) override;

private:
    core::IbProtDom* m_refProtDom;

    const uint32_t m_sendBufferSize;

    const uint16_t m_ibQPSize;
    const uint16_t m_ibCQSize;

    uint16_t m_numConnections;

    ibv_qp* m_ibQP;
    uint32_t m_physicalQPId;

    ibv_cq* m_ibSendCQ;
    ibv_cq* m_ibRecvCQ;

private:
    ibv_qp* __CreateQP(uint16_t size);
    ibv_cq* __CreateCQ(uint16_t size);
    
    void __SetInitStateQP();
    void __SetReadyToSendQP();
    void __SetReadyToRecvQP();
};

}
}

#endif
