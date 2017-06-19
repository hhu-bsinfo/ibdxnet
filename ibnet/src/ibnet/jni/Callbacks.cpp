#include "Callbacks.h"

#include <iostream>

namespace ibnet {
namespace jni {

Callbacks::Callbacks(JNIEnv* env, jobject callbacks) :
    m_vm(nullptr),
    m_callbacks(env->NewGlobalRef(callbacks)),
    m_midNodeDiscovered(env->GetMethodID(env->GetObjectClass(callbacks), "nodeDiscovered", "(S)V")),
    m_midNodeInvalidated(env->GetMethodID(env->GetObjectClass(callbacks), "nodeInvalidated", "(S)V")),
    m_midNodeConnected(env->GetMethodID(env->GetObjectClass(callbacks), "nodeConnected", "(S)V")),
    m_midNodeDisconnected(
        env->GetMethodID(env->GetObjectClass(callbacks), "nodeDisconnected", "(S)V")),
    m_midGetReceiveBuffer(env->GetMethodID(env->GetObjectClass(callbacks), "getReceiveBuffer",
        "(I)Ljava/nio/ByteBuffer;")),
    m_midReceivedBuffer(env->GetMethodID(env->GetObjectClass(callbacks), "receivedBuffer",
        "(SLjava/nio/ByteBuffer;I)V")),
    m_midReceivedFlowControlData(env->GetMethodID(env->GetObjectClass(callbacks),
        "receivedFlowControlData", "(SI)V"))
{
    env->GetJavaVM(&m_vm);

    if (m_midNodeDiscovered == 0) {
        std::cout << "Could not find m_midNodeDiscovered callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }

    if (m_midNodeInvalidated == 0) {
        std::cout << "Could not find m_midNodeInvalidated callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }

    if (m_midNodeConnected == 0) {
        std::cout << "Could not find m_midNodeConnected callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }

    if (m_midNodeDisconnected == 0) {
        std::cout << "Could not find nodeDisconnected callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }

    if (m_midGetReceiveBuffer == 0) {
        std::cout << "Could not find m_midGetReceiveBuffer callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }

    if (m_midReceivedBuffer == 0) {
        std::cout << "Could not find receivedBuffer callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }

    if (m_midReceivedFlowControlData == 0) {
        std::cout << "Could not find receivedBuffer callback method"
                  << std::endl;
        // TODO proper exception derived from from sys
        throw std::runtime_error("");
    }
}

Callbacks::~Callbacks(void)
{

}

}
}