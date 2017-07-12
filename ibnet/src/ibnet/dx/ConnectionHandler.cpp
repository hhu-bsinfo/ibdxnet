#include "ConnectionHandler.h"

namespace ibnet {
namespace dx {

ConnectionHandler::ConnectionHandler(JNIEnv* env, jobject object,
        const std::vector<std::unique_ptr<RecvThread>>& recvThreads) :
    m_vm(nullptr),
    m_object(env->NewGlobalRef(object)),
    m_recvThreads(recvThreads),
    m_midNodeConnected(env->GetMethodID(env->GetObjectClass(object),
        "nodeConnected", "(S)V")),
    m_midNodeDisconnected(
        env->GetMethodID(env->GetObjectClass(object), "nodeDisconnected",
            "(S)V"))
{
    env->GetJavaVM(&m_vm);
}

ConnectionHandler::~ConnectionHandler(void)
{

}

}
}