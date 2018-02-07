//
// Created by nothaas on 2/1/18.
//

#ifndef IBNET_MSGRC_MSGRCJNIBINDINGCALLBACKHANDLER_H
#define IBNET_MSGRC_MSGRCJNIBINDINGCALLBACKHANDLER_H

#include <jni.h>

#include "ibnet/sys/JNIHelper.h"

#include "ibnet/msgrc/RecvHandler.h"
#include "ibnet/msgrc/SendHandler.h"

namespace ibnet {
namespace msgrc {

class MsgrcJNIBindingCallbackHandler
{
public:
    MsgrcJNIBindingCallbackHandler(JNIEnv* env, jobject object);

    ~MsgrcJNIBindingCallbackHandler() = default;

    inline void NodeDiscovered(con::NodeId nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = sys::JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeDiscovered, nodeId);
        sys::JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void NodeInvalidated(con::NodeId nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = sys::JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeInvalidated, nodeId);
        sys::JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void NodeDisconnected(con::NodeId nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = sys::JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeDisconnected, nodeId);
        sys::JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void Received(RecvHandler::ReceivedPackage* recvPackage)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = sys::JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midReceived, (jlong) recvPackage);
        sys::JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline const SendHandler::NextWorkPackage* GetNextDataToSend(
            const SendHandler::PrevWorkPackageResults* prevResults,
            const SendHandler::CompletedWorkList* completionList)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = sys::JNIHelper::GetEnv(m_vm);
        env->CallLongMethod(m_object, m_midGetNextDataToSend,
            (jlong) &m_nextWorkPackage, (jlong) prevResults,
            (jlong) completionList);
        sys::JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;

        return &m_nextWorkPackage;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    jmethodID m_midNodeDiscovered;
    jmethodID m_midNodeInvalidated;
    jmethodID m_midNodeDisconnected;

    jmethodID m_midReceived;
    jmethodID m_midGetNextDataToSend;

private:
    SendHandler::NextWorkPackage m_nextWorkPackage;
};

}
}

#endif //IBNET_MSGRC_MSGRCJNIBINDINGCALLBACKHANDLER_H