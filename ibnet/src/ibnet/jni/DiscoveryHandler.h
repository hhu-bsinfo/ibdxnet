#ifndef IBNET_JNI_DISCOVERYHANDLER_H
#define IBNET_JNI_DISCOVERYHANDLER_H

#include "ibnet/core/IbDiscoveryManager.h"

#include "JNIHelper.h"

namespace ibnet {
namespace jni {

class DiscoveryHandler : public ibnet::core::IbDiscoveryManager::Listener
{
public:
    DiscoveryHandler(JNIEnv* env, jobject object);
    ~DiscoveryHandler(void);

    void NodeDiscovered(uint16_t nodeId) override
    {
        __NodeDiscovered(nodeId);
    }

    void NodeInvalidated(uint16_t nodeId) override
    {
        __NodeInvalidated(nodeId);
    }

private:
    inline void __NodeDiscovered(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeDiscovered, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void __NodeInvalidated(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeInvalidated, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    jmethodID m_midNodeDiscovered;
    jmethodID m_midNodeInvalidated;
};

}
}

#endif //IBNET_JNI_DISCOVERYHANDLER_H
