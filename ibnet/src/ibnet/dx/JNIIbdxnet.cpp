/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf, Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "JNIIbdxnet.h"

#include <memory>

#include <backwards/backward.hpp>

#include "ibnet/sys/Debug.h"
#include "ibnet/sys/Logger.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/core/IbException.h"
#include "ibnet/core/IbNodeConfArgListReader.h"

#include "ConnectionCreator.h"
#include "ConnectionHandler.h"
#include "DebugThread.h"
#include "DiscoveryHandler.h"
#include "RecvBufferPool.h"
#include "SendBuffers.h"


// Notes about JNI performance:
// CPU: Intel® Core™ i7-5820K CPU @ 3.30GHz × 12 
// Avg. time per call from Java -> JNI: ~650 ns
// Avg. time per call from JNI -> Java (callbacks): ~14 ns

static std::unique_ptr<backward::SignalHandling> g_signalHandler;

static std::shared_ptr<ibnet::dx::ConnectionHandler> g_connectionHandler;
static std::shared_ptr<ibnet::dx::DiscoveryHandler> g_discoveryHandler;
static std::shared_ptr<ibnet::dx::RecvHandler> g_recvHandler;
static std::shared_ptr<ibnet::dx::SendHandler> g_sendHandler;

static std::shared_ptr<ibnet::core::IbDevice> g_device;
static std::shared_ptr<ibnet::core::IbProtDom> g_protDom;

static std::shared_ptr<ibnet::core::IbSharedRecvQueue> g_sharedRecvQueue;
static std::shared_ptr<ibnet::core::IbSharedRecvQueue> g_sharedFlowControlRecvQueue;
static std::shared_ptr<ibnet::core::IbCompQueue> g_sharedRecvCompQueue;
static std::shared_ptr<ibnet::core::IbCompQueue> g_sharedFlowControlRecvCompQueue;

static std::shared_ptr<ibnet::core::IbDiscoveryManager> g_discoveryManager;
static std::shared_ptr<ibnet::core::IbConnectionManager> g_connectionManager;

static std::shared_ptr<ibnet::dx::SendBuffers> g_sendBuffers;
static std::shared_ptr<ibnet::dx::RecvBufferPool> g_recvBufferPool;

static std::vector<std::unique_ptr<ibnet::dx::RecvThread>> g_recvThreads;
static std::vector<std::unique_ptr<ibnet::dx::SendThread>> g_sendThreads;

static std::unique_ptr<ibnet::dx::DebugThread> g_debugThread;

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_init(
        JNIEnv* p_env, jclass p_class, jshort p_ownNodeId, jint p_outBufferSize,
        jint p_maxRecvReqs, jint p_flowControlMaxRecvReqs,
        jint p_sendThreads, jint p_recvThreads, jint p_maxNumConnections,
        jobject p_sendHandler, jobject p_recvHandler,
        jobject p_discoveryHandler, jobject p_connectionHandler,
        jboolean p_enableSignalHandler, jboolean p_enableDebugThread)
{
    // setup foundation
    if (p_enableSignalHandler) {
        g_signalHandler = std::make_unique<backward::SignalHandling>();
    }

    ibnet::sys::Logger::Setup();

    IBNET_LOG_DEBUG("Foundation setup done");

    // callbacks to java vm

    try {
        g_connectionHandler = std::make_shared<ibnet::dx::ConnectionHandler>(
            p_env, p_connectionHandler, g_recvThreads);
        g_discoveryHandler = std::make_shared<ibnet::dx::DiscoveryHandler>(
            p_env, p_discoveryHandler);
        g_recvHandler = std::make_shared<ibnet::dx::RecvHandler>(p_env,
            p_recvHandler);
        g_sendHandler = std::make_shared<ibnet::dx::SendHandler>(p_env,
            p_sendHandler);
    } catch (...) {
        IBNET_LOG_ERROR("Setting up callbacks to java vm failed");
        return (jboolean) 0;
    }

    IBNET_LOG_DEBUG("Setting up java vm callbacks done");

    try {
        IBNET_LOG_INFO("Initializing infiniband backend...");

        g_device = std::make_shared<ibnet::core::IbDevice>();
        g_protDom = std::make_shared<ibnet::core::IbProtDom>(g_device,
            "jni_ibnet");

        IBNET_LOG_DEBUG("Protection domain:\n{}", *g_protDom);

        g_sharedRecvQueue = std::make_shared<ibnet::core::IbSharedRecvQueue>(
            g_protDom,
            p_maxRecvReqs);
        g_sharedFlowControlRecvQueue =
            std::make_shared<ibnet::core::IbSharedRecvQueue>(g_protDom,
                p_flowControlMaxRecvReqs);

        g_sharedRecvCompQueue = std::make_shared<ibnet::core::IbCompQueue>(
            g_device,
            p_maxRecvReqs);
        g_sharedFlowControlRecvCompQueue =
            std::make_shared<ibnet::core::IbCompQueue>(g_device,
                p_flowControlMaxRecvReqs);

        // add nodes later
        ibnet::core::IbNodeConf nodeConf;

        g_discoveryManager = std::make_shared<ibnet::core::IbDiscoveryManager>(
            p_ownNodeId,
            nodeConf, 5730, 500);

        g_connectionManager = std::make_shared<ibnet::core::IbConnectionManager>(
            p_ownNodeId,
            5731, p_maxNumConnections, g_device, g_protDom,
            g_discoveryManager,
            std::make_unique<ibnet::dx::ConnectionCreator>(p_maxRecvReqs,
                p_flowControlMaxRecvReqs, g_sharedRecvQueue,
                g_sharedRecvCompQueue, g_sharedFlowControlRecvQueue,
                g_sharedFlowControlRecvCompQueue));

        g_connectionManager->SetNodeConnectedListener(g_connectionHandler.get());
        g_discoveryManager->SetNodeDiscoveryListener(g_discoveryHandler.get());

        IBNET_LOG_INFO("Initializing buffer pools...");

        g_sendBuffers = std::make_shared<ibnet::dx::SendBuffers>(
            p_outBufferSize, p_maxNumConnections, g_protDom);
        g_recvBufferPool = std::make_shared<ibnet::dx::RecvBufferPool>(
            p_outBufferSize, p_flowControlMaxRecvReqs, g_protDom);

        IBNET_LOG_INFO("Starting {} receiver threads", p_recvThreads);

        for (uint8_t i = 0; i < p_recvThreads; i++) {
            auto thread = std::make_unique<ibnet::dx::RecvThread>(i == 0,
                g_connectionManager, g_sharedRecvCompQueue,
                g_sharedFlowControlRecvCompQueue, g_recvBufferPool,
                g_recvHandler);
            thread->Start();
            g_recvThreads.push_back(std::move(thread));
        }

        IBNET_LOG_INFO("Starting {} sender threads", p_sendThreads);
        for (uint8_t i = 0; i < p_sendThreads; i++) {
            auto thread = std::make_unique<ibnet::dx::SendThread>(
                g_sendBuffers, g_sendHandler, g_connectionManager);
            thread->Start();
            g_sendThreads.push_back(std::move(thread));
        }
    } catch (...) {
        IBNET_LOG_ERROR("Initializing infiniband backend failed");

        // TODO shutdown what's created and initialized so far
        ibnet::sys::Logger::Shutdown();
        return (jboolean) 0;
    }

    // TODO adjust debug thread
    if (p_enableDebugThread) {
        g_debugThread = std::make_unique<ibnet::dx::DebugThread>(
            g_recvThreads, g_sendThreads);
        g_debugThread->Start();
    }

    IBNET_LOG_INFO("Initializing ibdxnet subsystem done");

	return (jboolean) 1;
}

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_shutdown(
        JNIEnv* p_env, jclass p_class)
{
    IBNET_LOG_TRACE_FUNC;

    jboolean res = (jboolean) 1;

    if (g_debugThread) {
        g_debugThread->Stop();
        g_debugThread.reset();
    }

    // TODO cleanup outgoing msg queues, make sure everything's processed?
    // TODO don't allow any new messages to be put to the send queue
    // wait until everything on the send queues is sent?

    for (auto& it : g_sendThreads) {
        it->Stop();
    }
    g_sendThreads.clear();

    for (auto& it : g_recvThreads) {
        it->Stop();
    }
    g_recvThreads.clear();

    try {
        g_connectionManager.reset();
        g_discoveryManager.reset();
        g_sharedRecvCompQueue.reset();
        g_sharedRecvQueue.reset();
        g_protDom.reset();
        g_device.reset();
    } catch (...) {
        res = (jboolean) 0;
    }

    g_connectionHandler.reset();
    g_discoveryHandler.reset();
    g_recvHandler.reset();
    g_sendHandler.reset();

    ibnet::sys::Logger::Shutdown();

    g_signalHandler.reset();

    return res;
}

JNIEXPORT void JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_addNode(
        JNIEnv* p_env, jclass p_class, jint p_ipv4)
{
    IBNET_LOG_TRACE_FUNC;

    ibnet::core::IbNodeConf::Entry entry(ibnet::sys::AddressIPV4((uint32_t) p_ipv4));
    g_discoveryManager->AddNode(entry);
}

JNIEXPORT jlong JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_getSendBufferAddress(
        JNIEnv* p_env, jclass p_class, jshort p_targetNodeId)
{
    std::shared_ptr<ibnet::core::IbConnection> connection =
        g_connectionManager->GetConnection((uint16_t) (p_targetNodeId & 0xFFFF));

    if (!connection) {
        return (jlong) -1;
    }

    ibnet::core::IbMemReg* buffer = g_sendBuffers->GetBuffer(
        connection->GetConnectionId());

    g_connectionManager->ReturnConnection(connection);

    return (jlong) buffer->GetAddress();
}

JNIEXPORT void JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_returnRecvBuffer(
        JNIEnv* p_env, jclass p_class, jlong p_addr)
{
    IBNET_LOG_TRACE("Return recv buffer {}", p_addr);

    ibnet::core::IbMemReg* mem = (ibnet::core::IbMemReg*) p_addr;
    g_recvBufferPool->ReturnBuffer(mem);
}