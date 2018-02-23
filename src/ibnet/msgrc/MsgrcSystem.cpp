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

#include <ibnet/sys/IllegalStateException.h>
#include "MsgrcSystem.h"

#include "ibnet/sys/Random.h"
#include "ibnet/sys/SystemInfo.h"

namespace ibnet {
namespace msgrc {

MsgrcSystem::MsgrcSystem() :
    m_configuration(nullptr),
    m_signalHandler(nullptr),
    m_device(nullptr),
    m_protDom(nullptr),
    m_discoveryManager(nullptr),
    m_exchangeManager(nullptr),
    m_jobManager(nullptr),
    m_recvBufferPool(nullptr),
    m_statisticsManager(nullptr),
    m_connectionManager(nullptr),
    m_recvDispatcher(nullptr),
    m_sendDispatcher(nullptr),
    m_executionEngine(nullptr)
{

}

MsgrcSystem::~MsgrcSystem()
{

}

void MsgrcSystem::Init()
{
    if (!m_configuration) {
        throw sys::IllegalStateException("Configuration null");
    }

    // setup foundation
    if (m_configuration->m_enableSignalHandler) {
        m_signalHandler = new backward::SignalHandling();
    }

    sys::Random::Init();
    sys::Logger::Setup();

    IBNET_LOG_INFO("Initializing...");

    sys::SystemInfo::LogHardwareReport();
    sys::SystemInfo::LogOSReport();
    sys::SystemInfo::LogApplicationReport();

    IBNET_LOG_DEBUG("%s", *m_configuration);

    m_device = new ibnet::core::IbDevice();
    m_protDom = new ibnet::core::IbProtDom(*m_device, "MsgrcLoopbackTest");

    IBNET_LOG_DEBUG("Protection domain:\n%s", *m_protDom);

    m_exchangeManager = new con::ExchangeManager(
        m_configuration->m_ownNodeId, m_configuration->m_portDiscMan);
    m_jobManager = new con::JobManager();

    m_discoveryManager = new con::DiscoveryManager(
        m_configuration->m_ownNodeId, m_configuration->m_nodeConfig,
        m_exchangeManager, m_jobManager);
    m_discoveryManager->SetListener(this);

    m_recvBufferPool = new dx::RecvBufferPool(
        m_configuration->m_recvBufferPoolSizeBytes,
        m_configuration->m_recvBufferSize, m_protDom);

    m_statisticsManager = new stats::StatisticsManager(
        m_configuration->m_statisticsThreadPrintIntervalMs);

    m_connectionManager = new ConnectionManager(
        m_configuration->m_ownNodeId, m_configuration->m_nodeConfig,
        m_configuration->m_connectionCreationTimeoutMs,
        m_configuration->m_maxNumConnections, m_device, m_protDom,
        m_exchangeManager, m_jobManager, m_discoveryManager,
        m_configuration->m_sendBufferSize, m_configuration->m_SQSize,
        m_configuration->m_SRQSize, m_configuration->m_sharedSCQSize,
        m_configuration->m_sharedRCQSize);

    m_connectionManager->SetListener(this);

    m_recvDispatcher = new RecvDispatcher(m_connectionManager,
        m_recvBufferPool, m_statisticsManager, this);

    m_sendDispatcher = new SendDispatcher(
        m_configuration->m_recvBufferSize, m_connectionManager,
        m_statisticsManager, this);

    m_executionEngine = new dx::ExecutionEngine(2, m_statisticsManager);

    m_executionEngine->AddExecutionUnit(0, m_sendDispatcher);
    m_executionEngine->AddExecutionUnit(1, m_recvDispatcher);

    if (m_configuration->m_pinSendRecvThreads) {
        m_executionEngine->PinWorker(0, 0);
        m_executionEngine->PinWorker(1, 1);
    }

    m_executionEngine->Start();

    if (m_configuration->m_statisticsThreadPrintIntervalMs > 0) {
        m_statisticsManager->Start();
    }

    _PostInit();

    IBNET_LOG_DEBUG("Initializing done");
}

void MsgrcSystem::Shutdown()
{
    IBNET_LOG_INFO("Shutting down...");

    m_executionEngine->Stop();

    m_connectionManager->SetListener(nullptr);
    m_discoveryManager->SetListener(nullptr);

    if (m_configuration->m_statisticsThreadPrintIntervalMs > 0) {
        m_statisticsManager->Stop();
    }

    m_statisticsManager->PrintStatistics();

    delete m_executionEngine;

    delete m_sendDispatcher;
    delete m_recvDispatcher;

    delete m_connectionManager;

    delete m_statisticsManager;

    delete m_recvBufferPool;

    delete m_discoveryManager;
    delete m_jobManager;
    delete m_exchangeManager;

    delete m_protDom;
    delete m_device;

    delete m_signalHandler;

    delete m_configuration;

    IBNET_LOG_DEBUG("Shutdown done");

    sys::Logger::Shutdown();
    sys::Random::Shutdown();
}

}
}