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

#ifndef IBNET_MSGRC_MSGRCJNIBINDINGCALLBACKHANDLER_H
#define IBNET_MSGRC_MSGRCJNIBINDINGCALLBACKHANDLER_H

#include <jni.h>

#include "ibnet/sys/JNIHelper.h"

#include "ibnet/msgrc/RecvHandler.h"
#include "ibnet/msgrc/SendHandler.h"

namespace ibnet {
namespace msgrc {

//
// Created by nothaas on 2/1/18.
//
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