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

#ifndef IBNET_MSGRC_CONNECTIONMANAGER_H
#define IBNET_MSGRC_CONNECTIONMANAGER_H

#include "ibnet/con/ConnectionManager.h"

#include "ibnet/dx/RecvBufferPool.h"

namespace ibnet {
namespace msgrc {

/**
 * Connection manager for reliable messaging using RC queue pairs
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 05.02.2018
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
     * @param ibSQSize Size of the send queue
     * @param ibSRQSize Size of the shared receive queue
     * @param ibSharedSCQSize Size of the shared send completion queue
     * @param ibSharedRCQSize Size of the shared receive completion queue
     */
    ConnectionManager(con::NodeId ownNodeId, const con::NodeConf& nodeConf,
            uint32_t connectionCreationTimeoutMs, uint32_t maxNumConnections,
            core::IbDevice* refDevice, core::IbProtDom* refProtDom,
            con::ExchangeManager* refExchangeManager,
            con::JobManager* refJobManager,
            con::DiscoveryManager* refDiscoveryManager, uint32_t sendBufferSize,
            uint16_t ibSQSize, uint16_t ibSRQSize, uint16_t ibSharedSCQSize,
            uint16_t ibSharedRCQSize);

    /**
     * Destructor
     */
    ~ConnectionManager() override;

    /**
     * Get the size of the send queue
     */
    uint16_t GetIbSQSize() const
    {
        return m_ibSQSize;
    }

    /**
     * Get the shared receive queue
     */
    ibv_srq* GetIbSRQ() const
    {
        return m_ibSRQ;
    }

    /**
     * Get the size of the shared receive queue
     */
    uint16_t GetIbSRQSize() const
    {
        return m_ibSRQSize;
    }

    /**
     * Get the shared send completion queue
     */
    ibv_cq* GetIbSharedSCQ() const
    {
        return m_ibSharedSCQ;
    }

    /**
     * Get the size of the shared send completion queue
     */
    uint16_t GetIbSharedSCQSize() const
    {
        return m_ibSharedSCQSize;
    }

    /**
     * Get the shared receive completion queue
     */
    ibv_cq* GetIbSharedRCQ() const
    {
        return m_ibSharedRCQ;
    }

    /**
     * Get the size of the shared receive completion queue
     */
    uint16_t GetIbSharedRCQSize() const
    {
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
