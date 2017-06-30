#ifndef IBNET_JNI_CALLBACKS_H
#define IBNET_JNI_CALLBACKS_H

#include <jni.h>
#include <string.h>

#include <cstdint>
#include <stdexcept>

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace jni {

class Callbacks
{
public:
    Callbacks(JNIEnv* env, jobject callbacks);
    ~Callbacks(void);

    inline void NodeDiscovered(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeDiscovered, nodeId);
        __ReturnEnv(env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void NodeInvalidated(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeInvalidated, nodeId);
        __ReturnEnv(env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void NodeConnected(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeConnected, nodeId);
        __ReturnEnv(env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void NodeDisconnected(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeDisconnected, nodeId);
        __ReturnEnv(env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void HandleReceive(uint16_t source, void* buffer, uint32_t length)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = __GetEnv();

        // get a byte buffer with at least length size
        jobject byteBuffer =
            env->CallObjectMethod(m_callbacks, m_midGetReceiveBuffer, length);

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
            env->CallVoidMethod(m_callbacks, m_midReceivedBuffer, source,
                byteBuffer, length);
        }

        __ReturnEnv(env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void HandleReceiveFlowControlData(uint16_t source, uint32_t data)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midReceivedFlowControlData, source,
            data);
        __ReturnEnv(env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_callbacks;
    jmethodID m_midNodeDiscovered;
    jmethodID m_midNodeInvalidated;
    jmethodID m_midNodeConnected;
    jmethodID m_midNodeDisconnected;
    jmethodID m_midGetReceiveBuffer;
    jmethodID m_midReceivedBuffer;
    jmethodID m_midReceivedFlowControlData;
    jfieldID m_directBufferAddressField;

    inline JNIEnv* __GetEnv(void)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env;

        // Very important note:
        // If the JVM is crashing here: Have a look at the JNINotes.md file

        int envStat = m_vm->GetEnv((void **)&env, JNI_VERSION_1_8);
        if (envStat == JNI_EDETACHED) {
            if (m_vm->AttachCurrentThread((void **) &env, NULL) != 0) {
                throw std::runtime_error("Failed to attach to java vm");
            }
        } else if (envStat == JNI_OK) {
            // already attached to environment
        } else if (envStat == JNI_EVERSION) {
            throw std::runtime_error("Failed to attach to java vm, jni version not supported");
        }

        return env;
    }

    inline void __ReturnEnv(JNIEnv* env)
    {
        IBNET_LOG_TRACE_FUNC;

        // Don't check for exceptions because this is extremely expensive
        // and kills performance on recv callbacks
        // if (env->ExceptionCheck()) {
        //    env->ExceptionDescribe();
        // }

        // Don't detach. This is very expensive and increases the costs
        // for re-attaching a lot. The number of threads calling back to
        // the java context is limited, so we keep them attached
        // m_vm->DetachCurrentThread();
    }
};

}
}

#endif //IBNET_JNI_CALLBACKS_H
