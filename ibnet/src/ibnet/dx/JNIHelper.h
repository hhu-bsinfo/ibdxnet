#ifndef IBNET_DX_JNIHELPER_H
#define IBNET_DX_JNIHELPER_H

#include <jni.h>
#include <string.h>

#include <cstdint>
#include <stdexcept>

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace dx {

class JNIHelper
{
public:
    static inline JNIEnv* GetEnv(JavaVM* vm)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env;

        // Very important note:
        // If the JVM is crashing here: Have a look at the JNINotes.md file

        int envStat = vm->GetEnv((void **)&env, JNI_VERSION_1_8);
        if (envStat == JNI_EDETACHED) {
            if (vm->AttachCurrentThread((void **) &env, NULL) != 0) {
                throw std::runtime_error("Failed to attach to java vm");
            }
        } else if (envStat == JNI_OK) {
            // already attached to environment
        } else if (envStat == JNI_EVERSION) {
            throw std::runtime_error("Failed to attach to java vm, jni version not supported");
        }

        return env;
    }

    static inline void ReturnEnv(JavaVM* vm, JNIEnv* env)
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
        // vm->DetachCurrentThread();
    }
    
    static inline jmethodID GetAndVerifyMethod(JNIEnv* env, jobject object,
            const std::string& name, const std::string& signature)
    {
        jmethodID mid;
        
        mid = env->GetMethodID(env->GetObjectClass(object), name.c_str(),
            signature.c_str());

        if (mid == 0) {
            IBNET_LOG_ERROR("Could not find method id of {}, {}",
                name, signature);
            throw std::runtime_error("Could not find method id of " + name +
                ", " + signature);
        }

        return mid;
    }

private:
    JNIHelper(void) {};
    ~JNIHelper(void) {};
};

}
}

#endif //IBNET_DX_JNIHELPER_H
