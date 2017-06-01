#ifndef IBNET_CORE_IBDISCOVERYMANAGER_H
#define IBNET_CORE_IBDISCOVERYMANAGER_H

#include <atomic>
#include <iostream>
#include <mutex>

#include "ibnet/sys/SocketUDP.h"
#include "ibnet/sys/ThreadLoop.h"

#include "IbNodeConf.h"
#include "IbNodeId.h"
#include "IbRemoteInfo.h"

namespace ibnet {
namespace core {

class IbDiscoveryManager : public ibnet::sys::ThreadLoop
{
public:
    class Listener
    {
    public:
        Listener(void) {};
        virtual ~Listener(void) {};

        virtual void NodeDiscovered(uint16_t nodeId) = 0;

        virtual void NodeInvalidated(uint16_t nodeId) = 0;
    };

    IbDiscoveryManager(uint16_t ownNodeId, const IbNodeConf& nodeConf, uint16_t udpPort, uint32_t discoveryIntervalMs);
    ~IbDiscoveryManager(void);

    void SetNodeDiscoveryListener(Listener* listener) {
        m_listener = listener;
    }

    void AddNode(const IbNodeConf::Entry& entry);

    const std::shared_ptr<IbNodeConf::Entry>& GetNodeInfo(uint16_t nodeId);

    // call this if the connection was lost
    void InvalidateNodeInfo(uint16_t nodeId);

protected:
    void _RunLoop(void) override;

private:
    uint16_t m_ownNodeId;
    std::vector<std::shared_ptr<IbNodeConf::Entry>> m_infoToGet;

    uint16_t m_socketPort;
    std::unique_ptr<sys::SocketUDP> m_socket;

    std::mutex m_lock;
    std::shared_ptr<IbNodeConf::Entry> m_nodeInfo[IbNodeId::MAX_NUM_NODES];

    Listener* m_listener;

    uint32_t m_discoveryIntervalMs;
    bool m_activePhase;

private:
    enum PaketId
    {
        e_PaketIdReq = 0,
        e_PaketIdInfo = 1,
    };

    struct PaketNodeInfo
    {
        uint32_t m_magic;
        uint32_t m_paketId;
        uint16_t m_nodeId;
    } __attribute__((packed));
};

}
}

#endif // IBNET_CORE_IBDISCOVERYMANAGER_H
