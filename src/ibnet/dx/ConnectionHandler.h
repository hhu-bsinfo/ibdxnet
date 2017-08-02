#ifndef IBNET_DX_CONNECTIONHANDLER_H
#define IBNET_DX_CONNECTIONHANDLER_H

#include "ibnet/core/IbConnectionManager.h"

#include "JNIHelper.h"
#include "RecvThread.h"

namespace ibnet {
namespace dx {

/**
 * Handle node connect/disconnects by calling back to the jvm space
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.07.2017
 */
class ConnectionHandler : public ibnet::core::IbConnectionManager::Listener
{
public:
    /**
     * Constructor
     *
     * @param env The java environment from a java thread
     * @param object Java object of the equivalent callback class in java
     * @param recvThread Pointer to the receive thread
     */
    ConnectionHandler(JNIEnv* env, jobject object,
        std::shared_ptr<RecvThread>& recvThread);
    ~ConnectionHandler(void);

    /**
     * Override
     */
    void NodeConnected(uint16_t nodeId,
                       ibnet::core::IbConnection &connection) override
    {
        m_recvThread->NodeConnected(connection);

        __NodeConnected(nodeId);
    }

    /**
     * Override
     */
    void NodeDisconnected(uint16_t nodeId) override
    {
        __NodeDisconnected(nodeId);
    }

private:
    inline void __NodeConnected(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeConnected, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void __NodeDisconnected(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeDisconnected, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    std::shared_ptr<RecvThread>& m_recvThread;

    jmethodID m_midNodeConnected;
    jmethodID m_midNodeDisconnected;
};

}
}

#endif //IBNET_DX_CONNECTIONHANDLER_H
