//
// Created by nothaas on 1/31/18.
//

#ifndef IBNET_MSGRC_MSGRCSYSTEM_H
#define IBNET_MSGRC_MSGRCSYSTEM_H

#include "ibnet/sys/IllegalStateException.h"

#include "ibnet/core/IbDevice.h"
#include "ibnet/core/IbProtDom.h"

#include "ibnet/con/ConnectionListener.h"
#include "ibnet/con/DiscoveryManager.h"
#include "ibnet/con/DiscoveryListener.h"
#include "ibnet/con/ExchangeManager.h"
#include "ibnet/con/JobManager.h"
#include "ibnet/con/NodeConf.h"

#include "ibnet/dx/ExecutionEngine.h"
#include "ibnet/dx/RecvBufferPool.h"

#include "ibnet/stats/StatisticsManager.h"

#include "ibnet/msgrc/ConnectionManager.h"
#include "ibnet/msgrc/RecvDispatcher.h"
#include "ibnet/msgrc/RecvHandler.h"
#include "ibnet/msgrc/SendDispatcher.h"
#include "ibnet/msgrc/SendHandler.h"

namespace ibnet {
namespace msgrc {

class MsgrcSystem : public con::DiscoveryListener,
        public con::ConnectionListener, public RecvHandler, public SendHandler
{
public:
    struct Configuration
    {
        bool m_pinSendRecvThreads = true;
        bool m_enableSignalHandler = true;
        uint32_t m_statisticsThreadPrintIntervalMs = 0;
        con::NodeId m_ownNodeId = ibnet::con::NODE_ID_INVALID;
        uint16_t m_portDiscMan = 5730;
        ibnet::con::NodeConf m_nodeConfig = {};
        uint32_t m_connectionCreationTimeoutMs = 5000;
        uint16_t m_maxNumConnections = 100;
        uint16_t m_SQSize = 20;
        uint16_t m_SRQSize = m_maxNumConnections * m_SQSize;
        uint16_t m_sharedSCQSize = m_SRQSize;
        uint16_t m_sharedRCQSize = m_SRQSize;
        uint32_t m_sendBufferSize = 1024 * 1024 * 4;
        uint64_t m_recvBufferPoolSizeBytes =
            static_cast<uint64_t>(1024 * 1024 * 1024 * 2ll);
        uint32_t m_recvBufferSize = 1024 * 16;

        friend std::ostream &operator<<(std::ostream& os,
                const Configuration& o) {
            return os << "MsgrcSystem Configuration:" << std::endl <<
                "m_pinSendRecvThreads: " << o.m_pinSendRecvThreads <<
                    std::endl <<
                "m_enableSignalHandler: " << o.m_enableSignalHandler <<
                    std::endl <<
                "m_statisticsThreadPrintIntervalMs: " <<
                    o.m_statisticsThreadPrintIntervalMs << std::endl <<
                "m_ownNodeId: " << std::hex << o.m_ownNodeId << std::endl <<
                "m_portDiscMan: " << std::dec << o.m_portDiscMan << std::endl <<
                "m_nodeConfig: " << o.m_nodeConfig << std::endl <<
                "m_connectionCreationTimeoutMs: " <<
                    o.m_connectionCreationTimeoutMs << std::endl <<
                "m_maxNumConnections: " << o.m_maxNumConnections << std::endl <<
                "m_SQSize: " << o.m_SQSize << std::endl <<
                "m_SRQSize: " << o.m_SRQSize << std::endl <<
                "m_sharedSCQSize: " << o.m_sharedSCQSize << std::endl <<
                "m_sharedRCQSize: " << o.m_sharedRCQSize << std::endl <<
                "m_sendBufferSize: " << o.m_sendBufferSize << std::endl <<
                "m_recvBufferPoolSizeBytes: " << o.m_recvBufferPoolSizeBytes <<
                    std::endl <<
                "m_recvBufferSize: " << o.m_recvBufferSize << std::endl;
        }
    };

public:
    MsgrcSystem();

    virtual ~MsgrcSystem();

    void Init();

    void Shutdown();

protected:
    Configuration* m_configuration;

    void _SetConfiguration(Configuration* configuration) {
        if (m_configuration) {
            throw sys::IllegalStateException("Configuration already set");
        }

        m_configuration = configuration;
    }

    virtual void _PostInit() {};

protected:
    backward::SignalHandling* m_signalHandler;

    ibnet::core::IbDevice* m_device;
    ibnet::core::IbProtDom* m_protDom;

    ibnet::con::DiscoveryManager* m_discoveryManager;
    ibnet::con::ExchangeManager* m_exchangeManager;
    ibnet::con::JobManager* m_jobManager;

    ibnet::dx::RecvBufferPool* m_recvBufferPool;

    ibnet::stats::StatisticsManager* m_statisticsManager;

    ibnet::msgrc::ConnectionManager* m_connectionManager;
    ibnet::msgrc::RecvDispatcher* m_recvDispatcher;
    ibnet::msgrc::SendDispatcher* m_sendDispatcher;

    ibnet::dx::ExecutionEngine* m_executionEngine;
};

}
}

#endif //IBNET_MSGRC_MSGRCSYSTEM_H
