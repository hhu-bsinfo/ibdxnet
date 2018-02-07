//
// Created by nothaas on 2/1/18.
//

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
