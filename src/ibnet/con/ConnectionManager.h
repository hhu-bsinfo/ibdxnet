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

#ifndef IBNET_CON_CONNECTIONMANAGER_H
#define IBNET_CON_CONNECTIONMANAGER_H

#include "ibnet/core/IbDevice.h"
#include "ibnet/core/IbProtDom.h"

#include "Connection.h"
#include "ConnectionListener.h"
#include "DiscoveryManager.h"
#include "ExchangeManager.h"
#include "NodeConf.h"
#include "NodeId.h"
#include "JobManager.h"

namespace ibnet {
namespace con {

class ConnectionManager : public ExchangeDispatcher, JobDispatcher
{
public:
    /**
     * Constructor
     *
     * @param name Name of the connection manager
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
    ConnectionManager(const std::string& name, NodeId ownNodeId,
            const NodeConf& nodeConf, uint32_t connectionCreationTimeoutMs,
            uint32_t maxNumConnections, core::IbDevice* refDevice,
            core::IbProtDom* refProtDom, ExchangeManager* refExchangeManager,
            JobManager* refJobManager, DiscoveryManager* refDiscoveryManager);

    /**
     * Destructor
     */
    virtual ~ConnectionManager();

    /**
     * Get the max number of simultaneous connections to handle
     */
    uint16_t GetMaxNumConnections() const
    {
        return m_maxNumConnections;
    }

    /**
     * Set a connection listener which listens to node connect/disconnect
     * events
     *
     * @param listener Listener to set
     */
    void SetListener(ConnectionListener* refListener)
    {
        m_listener = refListener;
    }

    /**
     * Check if a connection is available (open and connected)
     *
     * @param destination Remote node id
     * @return True if available, false otherwise
     */
    bool IsConnectionAvailable(NodeId nodeId)
    {
        return m_connectionStates[nodeId].m_available.load(
                std::memory_order_relaxed) >= ConnectionState::CONNECTION_AVAILABLE;
    }

    /**
     * Get a connection
     *
     * If called for the first time with a node id previously not used, this might establish the
     * connection to the specified node and keeps it opened either until
     * closed or the max number of connections is exceeded and the connection
     * must be closed to make space for a new connection.
     *
     * @param nodeId Get the connection of the specified node
     * @return If successful, a reference to the established connection.
     *         The caller does not have to manage the memory.
     *         Throws exceptions on errors.
     */
    Connection* GetConnection(NodeId nodeId);

    /**
     * Return a connection that was retrieved on the GetConnection call.
     * This MUST be called for every GetConnection call to ensure
     * consistency when keeping track of currently used connections.
     *
     * @param connection Connection to return
     */
    void ReturnConnection(Connection* connection);

    /**
     * Explicitly close a connection
     *
     * @param nodeId Node id of the connection to close
     * @param force True to force close (don't wait for queues to be emptied),
     *          false to empty the queues and ensure everything queued is still
     *          being processed
     */
    void CloseConnection(NodeId nodeId, bool force);

protected:
    NodeId _GetOwnNodeId() const
    {
        return m_ownNodeId;
    }

    core::IbDevice* _GetRefDevice() const
    {
        return m_refDevice;
    }

    core::IbProtDom* _GetRefProtDom() const
    {
        return m_refProtDom;
    }

protected:
    void _DispatchExchangeData(uint32_t sourceIPV4,
            const ExchangeManager::PaketHeader* paketHeader,
            const void* data) override;

    void _DispatchJob(const JobQueue::Job* job) override;

protected:
    virtual Connection* _CreateConnection(ConnectionId connectionId) = 0;

    virtual void _ConnectionOpened(Connection& connection)
    {
    };

    virtual void _ConnectionClosed(NodeId nodeId)
    {
    };

private:
    struct JobCreateConnection : public JobQueue::Job
    {
        const NodeId m_targetNodeId;

        JobCreateConnection(JobQueue::JobType type, NodeId targetNodeId) :
                JobQueue::Job(type),
                m_targetNodeId(targetNodeId)
        {
        }
    };

    struct JobConnectConnection : public JobQueue::Job
    {
        const RemoteConnectionHeader m_remoteConnectionHeader;
        const uint8_t* m_remoteConnectionData;
        const size_t m_remoteConnectionDataSize;

        JobConnectConnection(JobQueue::JobType type,
                const RemoteConnectionHeader& remoteConnectionHeader,
                const uint8_t* remoteConnectionData,
                size_t remoteConnectionDataSize) :
                JobQueue::Job(type),
                m_remoteConnectionHeader(remoteConnectionHeader),
                m_remoteConnectionData(remoteConnectionData),
                m_remoteConnectionDataSize(remoteConnectionDataSize)
        {
        }

        ~JobConnectConnection() override
        {
            delete[] m_remoteConnectionData;
        }
    };

    struct JobCloseConnection : public JobQueue::Job
    {
        const NodeId m_nodeId;
        const bool m_force;
        const bool m_shutdown;

        JobCloseConnection(JobQueue::JobType type, NodeId nodeId,
                bool force, bool shutdown) :
                JobQueue::Job(type),
                m_nodeId(nodeId),
                m_force(force),
                m_shutdown(shutdown)
        {
        }
    };

private:
    const std::string m_name;
    const NodeId m_ownNodeId;
    const uint32_t m_connectionCreationTimeoutMs;
    const uint16_t m_maxNumConnections;

    const uint32_t m_connectionCtxIdent;

    core::IbDevice* m_refDevice;
    core::IbProtDom* m_refProtDom;

    ExchangeManager* m_refExchangeManager;
    JobManager* m_refJobManager;
    DiscoveryManager* m_refDiscoveryManager;

private:
    ConnectionListener* m_listener;

    ConnectionState m_connectionStates[NODE_ID_MAX_NUM_NODES];
    Connection* m_connections[NODE_ID_MAX_NUM_NODES];
    uint16_t m_openConnections;

    std::vector<ConnectionId> m_availableConnectionIds;

    std::atomic<bool> m_flagShutdown;

private:
    ExchangeManager::PaketType m_conDataExchgPaketType;

    JobQueue::JobType m_createConnectionJobType;
    JobQueue::JobType m_connectConnectionJobType;
    JobQueue::JobType m_closeConnectionJobType;

    void __AddJobCreateConnection(NodeId destNodeId);

    void __AddJobConnectConnection(
            const RemoteConnectionHeader& remoteConnectionHeader,
            const uint8_t remoteConnectionData[], size_t remoteConnectionDatSize);

    void __AddJobCloseConnection(NodeId nodeId, bool force, bool shutdown);

    void __JobDispatchCreateConnection(const JobCreateConnection& job);

    void __JobDispatchConnectConnection(const JobConnectConnection& job);

    void __JobDispatchCloseConnection(const JobCloseConnection& job);

    bool __AllocateConnection(NodeId remoteNodeId);

    void __SendConnectionExchgData(NodeId remoteNodeId,
            uint8_t exchgFalgs, uint8_t exchgFlagsRemote, uint32_t remoteNodeIPV4);
};

}
}

#endif //IBNET_CON_CONNECTIONMANAGER_H
