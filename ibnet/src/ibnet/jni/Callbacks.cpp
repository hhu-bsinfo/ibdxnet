#include "Callbacks.h"

#include <iostream>

namespace ibnet {
namespace jni {

Callbacks::Callbacks(JNIEnv* env, jobject callbacks) :
    m_vm(nullptr),
    m_callbacks(env->NewGlobalRef(callbacks)),
    m_class(env->GetObjectClass(callbacks)),
    m_midNodeDiscovered(env->GetMethodID(m_class, "nodeDiscovered", "(S)V")),
    m_midNodeInvalidated(env->GetMethodID(m_class, "nodeInvalidated", "(S)V")),
    m_midNodeConnected(env->GetMethodID(m_class, "nodeConnected", "(S)V")),
    m_midNodeDisconnected(
        env->GetMethodID(m_class, "nodeDisconnected", "(S)V")),
    m_midGetReceiveBuffer(env->GetMethodID(m_class, "getReceiveBuffer",
        "(I)Ljava/nio/ByteBuffer;")),
    m_midReceivedBuffer(env->GetMethodID(m_class, "receivedBuffer",
        "(SLjava/nio/ByteBuffer;I)V")),
    m_midReceivedFlowControlData(env->GetMethodID(m_class,
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