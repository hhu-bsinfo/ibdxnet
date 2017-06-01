#ifndef IBNET_JNI_NODEDISCOVERYLISTENER_H
#define IBNET_JNI_NODEDISCOVERYLISTENER_H

#include "ibnet/core/IbDiscoveryManager.h"

#include "Callbacks.h"

namespace ibnet {
namespace jni {

class NodeDiscoveryListener : public ibnet::core::IbDiscoveryManager::Listener
{
public:
    NodeDiscoveryListener(std::shared_ptr<Callbacks> callbacks) :
        m_callbacks(callbacks)
    {
    };

    ~NodeDiscoveryListener(void)
    {
    };

    void NodeDiscovered(uint16_t nodeId) override
    {
        m_callbacks->NodeDiscovered(nodeId);
    }

    void NodeInvalidated(uint16_t nodeId) override
    {
        m_callbacks->NodeInvalidated(nodeId);
    }

private:
    std::shared_ptr<Callbacks> m_callbacks;
};

}
}

#endif //IBNET_JNI_NODEDISCOVERYLISTENER_H
