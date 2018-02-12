//
// Created by nothaas on 1/29/18.
//

#include "DiscoveryManager.h"

#include "ibnet/sys/Network.h"

#include "NodeNotAvailableException.h"

namespace ibnet {
namespace con {

DiscoveryManager::DiscoveryManager(NodeId ownNodeId, const NodeConf& nodeConf,
        ExchangeManager* refExchangeManager, JobManager* refJobManager) :
    m_ownNodeId(ownNodeId),
    m_refExchangeManager(refExchangeManager),
    m_refJobManager(refJobManager),
    m_listener(nullptr),
    m_lock(),
    m_infoToGet(),
    m_nodeInfo(),
    m_discoverReqExchgPaketType(m_refExchangeManager->GeneratePaketTypeId()),
    m_discoverRespExchgPaketType(m_refExchangeManager->GeneratePaketTypeId()),
    m_discoverJobType(m_refJobManager->GenerateJobTypeId()),
    m_discoverOnIdleJobType(m_refJobManager->GenerateJobTypeId()),
    m_discoveredJobType(m_refJobManager->GenerateJobTypeId())
{
    IBNET_LOG_INFO("Initializing node discovery list, own node id 0x%X...",
        ownNodeId);

    std::string ownHostname = sys::Network::GetHostname();

    for (auto& it : nodeConf.GetEntries()) {

        // don't add self
        if (it.GetHostname() != ownHostname) {
            m_infoToGet.push_back(new NodeConf::Entry(it));
        }
    }

    m_refExchangeManager->AddDispatcher(m_discoverReqExchgPaketType, this);
    m_refExchangeManager->AddDispatcher(m_discoverRespExchgPaketType, this);

    m_refJobManager->AddDispatcher(m_discoverJobType, this);
    m_refJobManager->AddDispatcher(m_discoverOnIdleJobType, this);
    m_refJobManager->AddDispatcher(m_discoveredJobType, this);

    // Run node discovery on job idle
    m_refJobManager->SetIdleJob(new JobQueue::Job(m_discoverOnIdleJobType));
}

DiscoveryManager::~DiscoveryManager()
{
    m_refJobManager->SetIdleJob(nullptr);

    m_refExchangeManager->RemoveDispatcher(m_discoverReqExchgPaketType, this);
    m_refExchangeManager->RemoveDispatcher(m_discoverRespExchgPaketType, this);

    m_refJobManager->RemoveDispatcher(m_discoverJobType, this);
    m_refJobManager->RemoveDispatcher(m_discoverOnIdleJobType, this);
    m_refJobManager->RemoveDispatcher(m_discoveredJobType, this);

    for (auto it: m_infoToGet) {
        delete it;
    }

    for (auto& it : m_nodeInfo) {

        if (it != nullptr) {
            delete it;
        }
    }
}

void DiscoveryManager::AddNode(const NodeConf::Entry& entry)
{
    IBNET_LOG_TRACE_FUNC;

    std::string ownHostname = sys::Network::GetHostname();

    // don't add ourselves
    if (entry.GetHostname() != ownHostname) {
        IBNET_LOG_INFO("Adding node %s", entry);

        std::lock_guard<std::mutex> l(m_lock);
        m_infoToGet.push_back(new NodeConf::Entry(entry));
    }

    // trigger initial discovery
    __JobAddDiscover();
}

const NodeConf::Entry& DiscoveryManager::GetNodeInfo(NodeId nodeId)
{
    IBNET_LOG_TRACE_FUNC;

    std::lock_guard<std::mutex> l(m_lock);

    if (!m_nodeInfo[nodeId]) {
        throw NodeNotAvailableException(nodeId);
    }

    return *m_nodeInfo[nodeId];
}

void DiscoveryManager::Invalidate(NodeId nodeId, bool shutdown)
{
    m_lock.lock();
    m_infoToGet.push_back(m_nodeInfo[nodeId]);
    m_nodeInfo[nodeId] = nullptr;
    m_lock.unlock();

    // add job to re-discover if system is not shutting down
    if (!shutdown) {
        __JobAddDiscover();
    }

    if (m_listener) {
        m_listener->NodeInvalidated(nodeId);
    }
}

void DiscoveryManager::_DispatchExchangeData(uint32_t sourceIPV4,
        const ExchangeManager::PaketHeader* paketHeader,
        const void* data)
{
    if (paketHeader->m_type == m_discoverReqExchgPaketType) {
        __ExchgSendDiscoveryResp(sourceIPV4);
    } else if (paketHeader->m_type == m_discoverRespExchgPaketType) {
        __JobAddDiscovered(sourceIPV4, paketHeader->m_sourceNodeId);
    }
}

void DiscoveryManager::_DispatchJob(const JobQueue::Job* job)
{
    if (job->m_type == m_discoverJobType) {
        __ExecuteDiscovery();

        // there are more nodes to be discovered
        if (m_infoToGet.size() != 0) {
            // avoid flooding the job manager
            // note: this slows down the job dispatching thread so following
            // jobs are delayed by 10 ms at least
            __JobAddDiscover();
        }
    } else if (job->m_type == m_discoverOnIdleJobType) {
        __ExecuteDiscovery();
    } else if (job->m_type == m_discoveredJobType) {
        auto* jobDiscovered = dynamic_cast<const JobDiscovered*>(job);

        bool found = false;

        m_lock.lock();

        // remove from processing list
        for (auto it = m_infoToGet.begin(); it != m_infoToGet.end(); it++) {
            if ((*it)->GetAddress().GetAddress() ==
                    jobDiscovered->m_targetIPV4) {
                IBNET_LOG_INFO("Discovered node %s as node id 0x%X",
                    (*it)->GetAddress().GetAddressStr(),
                    jobDiscovered->m_nodeIdDiscovered);

                // store remote node information
                m_nodeInfo[jobDiscovered->m_nodeIdDiscovered] = *it;

                m_infoToGet.erase(it);

                m_lock.unlock();

                found = true;

                // don't lock call to listener
                if (m_listener) {
                    m_listener->NodeDiscovered(
                        jobDiscovered->m_nodeIdDiscovered);
                }

                break;
            }
        }

        if (!found) {
            m_lock.unlock();
        }
    }
}

void DiscoveryManager::__ExchgSendDiscoveryReq(uint32_t destIPV4)
{
    m_refExchangeManager->SendData(m_discoverReqExchgPaketType, destIPV4);
}

void DiscoveryManager::__ExchgSendDiscoveryResp(uint32_t destIPV4)
{
    m_refExchangeManager->SendData(m_discoverRespExchgPaketType, destIPV4);
}

void DiscoveryManager::__JobAddDiscover()
{
    m_refJobManager->AddJob(new JobQueue::Job(m_discoverJobType));
}

void DiscoveryManager::__JobAddDiscovered(uint32_t sourceIPV4,
        NodeId sourceNodeIdDiscovered)
{
    m_refJobManager->AddJob(new JobDiscovered(m_discoveredJobType, sourceIPV4,
        sourceNodeIdDiscovered));
}

void DiscoveryManager::__ExecuteDiscovery()
{
    IBNET_LOG_TRACE("Requesting node info of %d nodes", m_infoToGet.size());

    m_lock.lock();

    // request remote node's information if not received, yet
    for (auto& it : m_infoToGet) {
        IBNET_LOG_TRACE("Requesting node info from %s",
            it->GetAddress().GetAddressStr());

        __ExchgSendDiscoveryReq(it->GetAddress().GetAddress());
    }

    m_lock.unlock();
}

}
}