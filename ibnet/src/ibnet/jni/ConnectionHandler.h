#ifndef IBNET_JNI_CONNECTIONHANDLER_H
#define IBNET_JNI_CONNECTIONHANDLER_H

#include "ibnet/core/IbConnectionManager.h"

#include "JNIHelper.h"
#include "RecvThread.h"

namespace ibnet {
namespace jni {

class ConnectionHandler : public ibnet::core::IbConnectionManager::Listener
{
public:
    ConnectionHandler(JNIEnv* env, jobject object,
        const std::vector<std::unique_ptr<ibnet::jni::RecvThread>>& recvThreads);
    ~ConnectionHandler(void);

    void NodeConnected(uint16_t nodeId,
                       ibnet::core::IbConnection &connection) override
    {
        for (auto& it : m_recvThreads) {
            it->NodeConnected(connection);
        }

        __NodeConnected(nodeId);
    }

    void NodeDisconnected(uint16_t nodeId) override
    {
        __NodeDisconnected(nodeId);
    }

private:
    inline void __NodeConnected(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeConnected, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void __NodeDisconnected(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeDisconnected, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    const std::vector<std::unique_ptr<ibnet::jni::RecvThread>>& m_recvThreads;

    jmethodID m_midNodeConnected;
    jmethodID m_midNodeDisconnected;
};

}
}

#endif //IBNET_JNI_CONNECTIONHANDLER_H
