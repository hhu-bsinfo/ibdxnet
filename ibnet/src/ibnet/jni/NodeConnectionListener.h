#ifndef IBNET_JNI_NODECONNECTIONLISTENER_H
#define IBNET_JNI_NODECONNECTIONLISTENER_H

#include "ibnet/core/IbConnectionManager.h"

#include "Callbacks.h"

namespace ibnet {
namespace jni {

class NodeConnectionListener : public ibnet::core::IbConnectionManager::Listener
{
public:
    NodeConnectionListener(std::shared_ptr<Callbacks> callbacks) :
        m_callbacks(callbacks)
    {
    };

    ~NodeConnectionListener(void)
    {
    };

    void NodeConnected(uint16_t nodeId,
                       ibnet::core::IbConnection &connection) override
    {
        m_callbacks->NodeConnected(nodeId);
    }

    void NodeDisconnected(uint16_t nodeId) override
    {
        m_callbacks->NodeDisconnected(nodeId);
    }

private:
    std::shared_ptr<Callbacks> m_callbacks;
};

}
}

#endif //IBNET_JNI_NODECONNECTIONLISTENER_H
