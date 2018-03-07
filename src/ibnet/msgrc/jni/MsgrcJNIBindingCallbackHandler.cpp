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

#include "MsgrcJNIBindingCallbackHandler.h"

namespace ibnet {
namespace msgrc {

MsgrcJNIBindingCallbackHandler::MsgrcJNIBindingCallbackHandler(JNIEnv* env,
        jobject object) :
        m_vm(nullptr),
        m_object(env->NewGlobalRef(object)),
        m_midNodeDiscovered(sys::JNIHelper::GetAndVerifyMethod(env, object,
                "nodeDiscovered", "(S)V")),
        m_midNodeInvalidated(sys::JNIHelper::GetAndVerifyMethod(env, object,
                "nodeInvalidated", "(S)V")),
        m_midNodeDisconnected(sys::JNIHelper::GetAndVerifyMethod(env, object,
                "nodeDisconnected", "(S)V")),
        m_midReceived(sys::JNIHelper::GetAndVerifyMethod(env, object,
                "received", "(J)V")),
        m_midGetNextDataToSend(sys::JNIHelper::GetAndVerifyMethod(env, object,
                "getNextDataToSend", "(JJJ)V")),
        m_nextWorkPackage()
{
    env->GetJavaVM(&m_vm);
}

}
}
