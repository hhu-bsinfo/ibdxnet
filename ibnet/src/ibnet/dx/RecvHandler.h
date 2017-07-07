#ifndef IBNET_DX_RECVHANDLER_H
#define IBNET_DX_RECVHANDLER_H

#include "JNIHelper.h"

namespace ibnet {
namespace dx {

class RecvHandler
{
public:
    RecvHandler(JNIEnv* env, jobject object);
    ~RecvHandler(void);

    inline void ReceivedBuffer(uint16_t source, void* buffer, uint32_t length)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);

        // get a byte buffer with at least length size
        jobject byteBuffer =
            env->CallObjectMethod(m_object, m_midGetReceiveBuffer, length);

        if (byteBuffer == NULL) {
            IBNET_LOG_ERROR("Getting buffer for received data failed, null");
        } else {
            // I can't believe these calls are so expensive they cut performance
            // to less than 1/20th...
            // void* byteBufferAddr = env->GetDirectBufferAddress(byteBuffer);
            // uint32_t byteBufferLength =
            //    (uint32_t) env->GetDirectBufferCapacity(byteBuffer);

            // that's better
            void* byteBufferAddr = (void*) (intptr_t)
                env->GetLongField(byteBuffer, m_directBufferAddressField);

            memcpy(byteBufferAddr, buffer, length);
            env->CallVoidMethod(m_object, m_midReceivedBuffer, source,
                byteBuffer, length);
        }

        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void ReceivedFlowControlData(uint16_t source, uint32_t data)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midReceivedFlowControlData, source,
            data);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    jmethodID m_midGetReceiveBuffer;
    jmethodID m_midReceivedBuffer;
    jmethodID m_midReceivedFlowControlData;

    jfieldID m_directBufferAddressField;
};

}
}

#endif //IBNET_DX_RECVHANDLER_H
