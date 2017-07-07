#include "DiscoveryHandler.h"

#include "JNIHelper.h"

namespace ibnet {
namespace jni {

DiscoveryHandler::DiscoveryHandler(JNIEnv* env, jobject object) :
    m_vm(nullptr),
    m_object(object),
    m_midNodeDiscovered(JNIHelper::GetAndVerifyMethod(env, object,
        "nodeDiscovered", "(S)V")),
    m_midNodeInvalidated(JNIHelper::GetAndVerifyMethod(env, object,
        "nodeInvalidated", "(S)V"))
{
    env->GetJavaVM(&m_vm);
}

DiscoveryHandler::~DiscoveryHandler(void)
{

}

}
}