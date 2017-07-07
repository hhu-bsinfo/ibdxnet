#include "RecvHandler.h"

namespace ibnet {
namespace dx {

RecvHandler::RecvHandler(JNIEnv* env, jobject object) :
    m_vm(nullptr),
    m_object(object),
    m_midGetReceiveBuffer(JNIHelper::GetAndVerifyMethod(env, m_object,
        "getReceiveBuffer", "(I)Ljava/nio/ByteBuffer;")),
    m_midReceivedBuffer(JNIHelper::GetAndVerifyMethod(env, m_object,
        "receivedBuffer", "(SLjava/nio/ByteBuffer;I)V")),
    m_midReceivedFlowControlData(JNIHelper::GetAndVerifyMethod(env, m_object,
        "receivedFlowControlData", "(SI)V")),
    m_directBufferAddressField(env->GetFieldID(
        env->FindClass("java/nio/Buffer"), "address", "J"))
{
    env->GetJavaVM(&m_vm);
}

RecvHandler::~RecvHandler(void)
{

}

}
}