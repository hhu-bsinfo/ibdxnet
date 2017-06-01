#ifndef IBNET_JNI_CALLBACKS_H
#define IBNET_JNI_CALLBACKS_H

#include <jni.h>
#include <string.h>

#include <cstdint>
#include <stdexcept>

namespace ibnet {
namespace jni {

class Callbacks
{
public:
    Callbacks(JNIEnv* env, jobject callbacks);
    ~Callbacks(void);

    inline void NodeDiscovered(uint16_t nodeId)
    {
        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeDiscovered, nodeId);
        __ReturnEnv(env);
    }

    inline void NodeInvalidated(uint16_t nodeId)
    {
        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeInvalidated, nodeId);
        __ReturnEnv(env);
    }

    inline void NodeConnected(uint16_t nodeId)
    {
        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeConnected, nodeId);
        __ReturnEnv(env);
    }

    inline void NodeDisconnected(uint16_t nodeId)
    {
        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midNodeConnected, nodeId);
        __ReturnEnv(env);
    }

    inline void HandleReceive(uint16_t source, void* buffer, uint32_t length)
    {
        JNIEnv* env = __GetEnv();

        jobject byteBuffer =
            env->CallObjectMethod(m_callbacks, m_midGetReceiveBuffer, length);

        void* byteBufferAddr = env->GetDirectBufferAddress(byteBuffer);
        uint32_t byteBufferLength =
            (uint32_t) env->GetDirectBufferCapacity(byteBuffer);

        if (byteBufferLength < length) {
            // TODO log error handling
            throw std::runtime_error("Recv buffer too small");
        }

        memcpy(byteBufferAddr, buffer, length);
        env->CallVoidMethod(m_callbacks, m_midReceivedBuffer, source,
            byteBuffer, length);

        __ReturnEnv(env);
    }

    inline void HandleReceiveFlowControlData(uint16_t source, uint32_t data)
    {
        JNIEnv* env = __GetEnv();
        env->CallVoidMethod(m_callbacks, m_midReceivedFlowControlData, source,
            data);
        __ReturnEnv(env);
    }

private:
    JavaVM* m_vm;
    jobject m_callbacks;
    jclass m_class;
    jmethodID m_midNodeDiscovered;
    jmethodID m_midNodeInvalidated;
    jmethodID m_midNodeConnected;
    jmethodID m_midNodeDisconnected;
    jmethodID m_midGetReceiveBuffer;
    jmethodID m_midReceivedBuffer;
    jmethodID m_midReceivedFlowControlData;

    inline JNIEnv* __GetEnv(void)
    {
        JNIEnv* env;

        int envStat = m_vm->GetEnv((void **)&env, JNI_VERSION_1_8);
        if (envStat == JNI_EDETACHED) {
            if (m_vm->AttachCurrentThread((void **) &env, NULL) != 0) {
                throw std::runtime_error("Failed to attach to java vm");
            }
        } else if (envStat == JNI_OK) {

        } else if (envStat == JNI_EVERSION) {
            throw std::runtime_error("Failed to attach to java vm, jni version not supported");
        }

        return env;
    }

    inline void __ReturnEnv(JNIEnv* env)
    {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
        }

        m_vm->DetachCurrentThread();
    }
};

}
}

#endif //IBNET_JNI_CALLBACKS_H
