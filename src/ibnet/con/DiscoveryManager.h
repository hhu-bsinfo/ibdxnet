//
// Created by nothaas on 1/29/18.
//

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

class DiscoveryManager : public ExchangeDispatcher, JobDispatcher
{
public:
    DiscoveryManager(NodeId ownNodeId, const NodeConf& nodeConf,
        ExchangeManager* refExchangeManager, JobManager* refJobManager);

    ~DiscoveryManager();

    void AddNode(const NodeConf::Entry& entry);

    const NodeConf::Entry& GetNodeInfo(NodeId nodeId);

    void SetListener(DiscoveryListener* listener) {
        m_listener = listener;
    }

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
        {}
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
