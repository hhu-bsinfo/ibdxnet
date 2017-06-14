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

#include "JNIIbnet.h"

#include <memory>

#include <backwards/backward.hpp>

#include "ibnet/sys/Logger.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/core/IbException.h"
#include "ibnet/core/IbNodeConfArgListReader.h"
#include "ibnet/msg/IbMessageSystem.h"

#include "Callbacks.h"
#include "MessageHandler.h"
#include "NodeConnectionListener.h"
#include "NodeDiscoveryListener.h"

// Notes about JNI performance:
// CPU: Intel® Core™ i7-5820K CPU @ 3.30GHz × 12 
// Avg. time per call from Java -> JNI: ~650 ns
// Avg. time per call from JNI -> Java (callbacks): ~14 ns

static std::shared_ptr<ibnet::jni::Callbacks> g_callbacks;
static std::shared_ptr<ibnet::jni::MessageHandler> g_messageHandler;
static std::shared_ptr<ibnet::jni::NodeConnectionListener> g_nodeConnectionListener;
static std::shared_ptr<ibnet::jni::NodeDiscoveryListener> g_nodeDiscoveryListener;
static std::unique_ptr<ibnet::msg::IbMessageSystem> g_messageSystem;

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbnet_init(JNIEnv* p_env,
        jclass p_class, jshort p_ownNodeId, jint p_maxRecvReqs,
        jint p_maxSendReqs, jint p_inOutBufferSize,
        jint p_flowControlMaxRecvReqs, jint p_flowControlMaxSendReqs,
        jint p_connectionJobPoolSize, jint p_sendThreads,
        jint p_recvThreads, jint p_maxNumConnections, jobject p_callbacks)
{
    try {
        g_callbacks = std::make_shared<ibnet::jni::Callbacks>(p_env, p_callbacks);
    } catch (...) {
        return (jboolean) 0;
    }

    backward::SignalHandling sh;

    ibnet::sys::Logger::Setup();

    g_messageHandler = std::make_shared<ibnet::jni::MessageHandler>(g_callbacks);
    g_nodeConnectionListener = std::make_shared<ibnet::jni::NodeConnectionListener>(g_callbacks);
    g_nodeDiscoveryListener = std::make_shared<ibnet::jni::NodeDiscoveryListener>(g_callbacks);

    ibnet::msg::Config config;
    config.m_maxRecvReqs = (uint16_t) p_maxRecvReqs;
    config.m_maxSendReqs = (uint16_t) p_maxSendReqs;
    config.m_inOutBufferSize = (uint32_t) p_inOutBufferSize;
    config.m_flowControlMaxRecvReqs = (uint16_t) p_flowControlMaxRecvReqs;
    config.m_flowControlMaxSendReqs = (uint16_t) p_flowControlMaxSendReqs;
    config.m_connectionJobPoolSize = (uint32_t) p_connectionJobPoolSize;
    config.m_sendThreads = (uint8_t) p_sendThreads;
    config.m_recvThreads = (uint8_t) p_recvThreads;
    config.m_maxNumConnections = (uint16_t) p_maxNumConnections;

    // add nodes later
    ibnet::core::IbNodeConf nodeConf;

    try {
        g_messageSystem = std::make_unique<ibnet::msg::IbMessageSystem>(
            p_ownNodeId, nodeConf, config, g_messageHandler,
            g_nodeConnectionListener, g_nodeDiscoveryListener);
    } catch (...) {
        ibnet::sys::Logger::Shutdown();
        return (jboolean) 0;
    }


//	int counter = 0;
//	auto enter = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<uint64_t, std::nano> m_total;
//
//	for (int i = 0; i < 1000000; i++) {
//		auto enter = std::chrono::high_resolution_clock::now();
//    	_Callbacks_nodeConnected(p_env, 1);
//		std::chrono::duration<uint64_t, std::nano> delta(std::chrono::high_resolution_clock::now() - enter);
//		m_total += delta;
//		counter++;
//	}
//
//	printf("callback to java: %f\n", std::chrono::duration<double>(m_total).count() / counter);

	return (jboolean) 1;
}

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbnet_shutdown(JNIEnv* p_env, jclass p_class)
{
    jboolean res = (jboolean) 1;

    try {
        g_messageSystem.reset();
    } catch (...) {
        res = (jboolean) 0;
    }

    g_messageHandler.reset();
    g_callbacks.reset();

    ibnet::sys::Logger::Shutdown();

    return res;
}

JNIEXPORT void JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbnet_addNode(JNIEnv* p_env,
        jclass p_class, jint p_ipv4)
{
    ibnet::core::IbNodeConf::Entry entry(ibnet::sys::AddressIPV4((uint32_t) p_ipv4));
    g_messageSystem->AddNode(entry);
}

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbnet_postBuffer(
        JNIEnv* p_env, jclass p_class, jshort p_nodeId, jobject p_buffer, jint p_length)
{
    // note: all buffers _MUST_ be allocated as direct buffers
    void* buf = p_env->GetDirectBufferAddress(p_buffer);
    // TODO copying is bad...but passing on the direct buffer is bad too =/
    void* tmp = malloc(p_length);
    memcpy(tmp, buf, (size_t) p_length);

    // buffer is free'd by message system (TODO bad idea?)
    jboolean ret = (jboolean) g_messageSystem->SendMessage((uint16_t) p_nodeId,
        tmp, (uint32_t) p_length);

    return ret;
}

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbnet_postFlowControlData(
    JNIEnv* p_env, jclass p_class, jshort p_nodeId, jint p_data)
{
    return (jboolean) g_messageSystem->SendFlowControl((uint16_t) p_nodeId,
        (uint32_t) p_data);
}