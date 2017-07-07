#include "SendHandler.h"

namespace ibnet {
namespace dx {

SendHandler::SendHandler(JNIEnv* env, jobject object) :
    m_vm(nullptr),
    m_object(object),
    m_midGetNextDataToSend(JNIHelper::GetAndVerifyMethod(env, object, "nodeDiscovered", "(SIJ)V"))
{
    env->GetJavaVM(&m_vm);
}

SendHandler::~SendHandler(void)
{

}

}
}