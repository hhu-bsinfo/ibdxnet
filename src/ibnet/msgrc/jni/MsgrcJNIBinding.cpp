//
// Created by on 1/31/18.
//

#include "MsgrcJNIBinding.h"

#include "MsgrcJNISystem.h"

static ibnet::msgrc::MsgrcJNISystem* g_system = nullptr;

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_init(
        JNIEnv* p_env, jclass p_class, jobject p_callbackHandler,
        jboolean p_pinSendRecvThreads, jboolean p_enableSignalHandler,
        jint p_statisticsThreadPrintIntervalMs, jshort p_ownNodeId,
        jint p_connectionCreationTimeoutMs, jint p_maxNumConnections,
        jint p_sqSize, jint p_srqSize, jint p_sharedSCQSize,
        jint p_sharedRCQSize, jint p_sendBufferSize,
        jlong p_recvBufferPoolSize, jint p_recvBufferSize)
{
    auto* configuration = new ibnet::msgrc::MsgrcSystem::Configuration();
    configuration->m_pinSendRecvThreads = p_pinSendRecvThreads;
    configuration->m_enableSignalHandler = p_enableSignalHandler;
    configuration->m_statisticsThreadPrintIntervalMs =
        static_cast<uint32_t>(p_statisticsThreadPrintIntervalMs);
    configuration->m_ownNodeId = static_cast<ibnet::con::NodeId>(p_ownNodeId);
    configuration->m_portDiscMan = 5730;
    // Nodes are added using the addNode function
    configuration->m_nodeConfig = ibnet::con::NodeConf();
    configuration->m_connectionCreationTimeoutMs =
        static_cast<uint32_t>(p_connectionCreationTimeoutMs);
    configuration->m_maxNumConnections =
        static_cast<uint16_t>(p_maxNumConnections);
    configuration->m_SQSize = static_cast<uint16_t>(p_sqSize);
    configuration->m_SRQSize = static_cast<uint16_t>(p_srqSize);
    configuration->m_sharedSCQSize = static_cast<uint16_t>(p_sharedSCQSize);
    configuration->m_sharedRCQSize = static_cast<uint16_t>(p_sharedRCQSize);
    configuration->m_sendBufferSize = static_cast<uint32_t>(p_sendBufferSize);
    configuration->m_recvBufferPoolSizeBytes =
        static_cast<uint64_t>(p_recvBufferPoolSize);
    configuration->m_recvBufferSize = static_cast<uint32_t>(p_recvBufferSize);

    try {
        g_system = new ibnet::msgrc::MsgrcJNISystem(configuration, p_env,
            p_callbackHandler);
        g_system->Init();
    } catch (ibnet::sys::Exception& e) {
        IBNET_LOG_ERROR("Exception on init: %s", e.what());
        delete g_system;
        return (jboolean) 0;
    }

    IBNET_LOG_INFO("Initializing done");

    return (jboolean) 1;
}

JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_shutdown(
        JNIEnv* p_env, jclass p_class)
{
    IBNET_LOG_INFO("Shutdown");

    g_system->Shutdown();
    delete g_system;
    g_system = nullptr;

    return (jboolean) 1;
}

JNIEXPORT void JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_addNode(
        JNIEnv* p_env, jclass p_class, jint p_ipv4)
{
    g_system->AddNode(static_cast<uint32_t>(p_ipv4));
}

JNIEXPORT jint JNICALL
Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_createConnection(JNIEnv* p_env,
        jclass p_class, jshort p_nodeId)
{
    return g_system->CreateConnection(static_cast<ibnet::con::NodeId>(p_nodeId));
}

JNIEXPORT jlong JNICALL
Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_getSendBufferAddress(JNIEnv* p_env,
        jclass p_class, jshort p_targetNodeId)
{
    return (jlong) g_system->GetSendBuffer(
        static_cast<ibnet::con::NodeId>(p_targetNodeId));
}

JNIEXPORT void JNICALL
    Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_returnRecvBuffer(JNIEnv* p_env,
        jclass p_class, jlong p_addr)
{
    g_system->ReturnRecvBuffer((ibnet::core::IbMemReg*) p_addr);
}
