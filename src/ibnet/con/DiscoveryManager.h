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

#ifndef IBNET_CON_DISCOVERYMANAGER_H
#define IBNET_CON_DISCOVERYMANAGER_H

#include <mutex>

#include "DiscoveryListener.h"
#include "ExchangeDispatcher.h"
#include "ExchangeManager.h"
#include "NodeConf.h"
#include "JobDispatcher.h"
#include "JobManager.h"

namespace ibnet {
namespace con {

/**
 * Statistic operation calculating a ratio of two units
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
class DiscoveryManager : public ExchangeDispatcher, JobDispatcher
{
public:
    /**
     * Constructor
     *
     * @param ownNodeId Node id of the current instance
     * @param nodeConf Node configuration to use
     * @param refExchangeManager Pointer to an exchange manager to use (memory managed by caller)
     * @param refJobManager Pointer to a job manager to use (memory managed by caller)
     */
    DiscoveryManager(NodeId ownNodeId, const NodeConf& nodeConf,
            ExchangeManager* refExchangeManager, JobManager* refJobManager);

    /**
     * Destructor
     */
    ~DiscoveryManager();

    /**
     * Add a new node entry to the node configuration.
     * This enables the new node to be discovred by the manager
     *
     * @param entry Entry to add to the manager
     */
    void AddNode(const NodeConf::Entry& entry);

    /**
     * Get discovery information about a specific node
     *
     * @param nodeId Node id of the node
     * @return A NodeConf Entry if the node was already discovered or
     *         NodeNotAvailableException if not available or not discovered, yet
     */
    const NodeConf::Entry& GetNodeInfo(NodeId nodeId);

    /**
     * Set a listener
     *
     * @param refListener Listener to set
     */
    void SetListener(DiscoveryListener* refListener)
    {
        m_listener = refListener;
    }

    /**
     * Invalidate a discovered node. This removes it from the discovered list
     * and re-lists it for future discovery
     *
     * @param nodeId Node id of node to invalidate
     * @param shutdown Set this to true if you call invalidate when shutting
     *        down your system/application to avoid re-discovering the node
     */
    void Invalidate(NodeId nodeId, bool shutdown);

protected:
    void _DispatchExchangeData(uint32_t sourceIPV4,
            const ExchangeManager::PaketHeader* paketHeader,
            const void* data) override;

    void _DispatchJob(const JobQueue::Job* job) override;

private:
    struct JobDiscovered : public JobQueue::Job
    {
        const uint32_t m_targetIPV4;
        const NodeId m_nodeIdDiscovered;

        JobDiscovered(JobQueue::JobType type, uint32_t targetIPV4,
                NodeId nodeIdDiscovered) :
                JobQueue::Job(type),
                m_targetIPV4(targetIPV4),
                m_nodeIdDiscovered(nodeIdDiscovered)
        {
        }
    };

private:
    const NodeId m_ownNodeId;

    ExchangeManager* m_refExchangeManager;
    JobManager* m_refJobManager;

    DiscoveryListener* m_listener;

    std::mutex m_lock;
    std::vector<NodeConf::Entry*> m_infoToGet;
    NodeConf::Entry* m_nodeInfo[NODE_ID_MAX_NUM_NODES];

private:
    ExchangeManager::PaketType m_discoverReqExchgPaketType;
    ExchangeManager::PaketType m_discoverRespExchgPaketType;

    JobQueue::JobType m_discoverJobType;
    JobQueue::JobType m_discoverOnIdleJobType;
    JobQueue::JobType m_discoveredJobType;

    void __ExchgSendDiscoveryReq(uint32_t destIPV4);

    void __ExchgSendDiscoveryResp(uint32_t destIPV4);

    void __JobAddDiscover();

    void __JobAddDiscovered(uint32_t sourceIPV4, NodeId sourceNodeIdDiscovered);

    void __ExecuteDiscovery();
};

}
}

#endif //IBNET_CON_DISCOVERYMANAGER_H
