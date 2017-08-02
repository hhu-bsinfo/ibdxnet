#include "ConnectionHandler.h"

namespace ibnet {
namespace dx {

ConnectionHandler::ConnectionHandler(JNIEnv* env, jobject object,
        std::shared_ptr<RecvThread>& recvThread) :
    m_vm(nullptr),
    m_object(env->NewGlobalRef(object)),
    m_recvThread(recvThread),
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