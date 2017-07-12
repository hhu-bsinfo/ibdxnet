#include "SendHandler.h"

namespace ibnet {
namespace dx {

SendHandler::SendHandler(JNIEnv* env, jobject object) :
    m_vm(nullptr),
    m_object(env->NewGlobalRef(object)),
    m_midGetNextDataToSend(JNIHelper::GetAndVerifyMethod(env, object,
        "getNextDataToSend", "(SI)J"))
{
    env->GetJavaVM(&m_vm);
}

SendHandler::~SendHandler(void)
{

}

}
}