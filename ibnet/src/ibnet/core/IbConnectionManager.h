#ifndef IBNET_CORE_IBCONNECTIONMANAGER_H
#define IBNET_CORE_IBCONNECTIONMANAGER_H

#include <atomic>
#include <mutex>
#include <unordered_map>

#include "ibnet/sys/SocketUDP.h"
#include "ibnet/sys/ThreadLoop.h"

#include "IbCompQueue.h"
#include "IbConnection.h"
#include "IbConnectionCreator.h"
#include "IbDiscoveryManager.h"
#include "IbNodeConf.h"
#include "IbNodeId.h"

namespace ibnet {
namespace core {

class IbConnectionManager : public ibnet::sys::ThreadLoop
{
public:
    class Listener
    {
    public:
        Listener(void) {};
        virtual ~Listener(void) {};

        virtual void NodeConnected(uint16_t nodeId, IbConnection& connection) = 0;

        virtual void NodeDisconnected(uint16_t nodeId) = 0;
    };

    IbConnectionManager(
            uint16_t ownNodeId,
            uint16_t socketPort,
            uint32_t maxNumConnections,
            std::shared_ptr<IbDevice>& device,
            std::shared_ptr<IbProtDom>& protDom,
            std::shared_ptr<IbDiscoveryManager>& discoveryManager,
            std::unique_ptr<IbConnectionCreator> connectionCreator);
    ~IbConnectionManager(void);

    void SetNodeConnectedListener(Listener* listener) {
        m_listener = listener;
    }

    uint16_t GetNodeIdForPhysicalQPNum(uint32_t qpNum) {
        auto it = m_qpNumToNodeIdMappings.find(qpNum);

        if (it != m_qpNumToNodeIdMappings.end()) {
            return it->second;
        }

        return IbNodeId::INVALID;
    }

    // use the return shared pointer as a handle to determine
    // who is still owning a reference to the connection
    // do not keep/store the shared pointer here!
    std::shared_ptr<IbConnection> GetConnection(uint16_t nodeId);

    void CloseConnection(uint16_t nodeId, bool force);

    friend std::ostream &operator<<(std::ostream& os, const IbConnectionManager& o) {
        os << "Connections (" << o.m_openConnections << "):";

        for (uint32_t i = 0; i < IbNodeId::MAX_NUM_NODES; i++) {
            if (o.m_connections[i] != nullptr) {
                os << std::endl << "Node 0x" << std::hex << i << ": " <<
                *(o.m_connections[i]);
            }
        }

        return os;
    }

protected:
    void _BeforeRunLoop(void) override;

    void _RunLoop(void) override;

    void _AfterRunLoop(void) override;

private:
    static const uint32_t MAX_CONNECT_RETIRES = 10;
    static const uint32_t CONNECT_RETRY_WAIT_MS;
    static const uint32_t MAX_QPS_PER_CONNECTION = 32;

private:
    sys::SocketUDP m_socket;

    uint32_t m_maxNumConnections;

    std::shared_ptr<IbDevice> m_device;
    std::shared_ptr<IbProtDom> m_protDom;
    std::shared_ptr<IbDiscoveryManager> m_discoveryManager;
    std::unique_ptr<IbConnectionCreator> m_connectionCreator;

    uint16_t m_ownNodeId;

    std::mutex m_connectionMutex;
    std::shared_ptr<IbConnection> m_connections[IbNodeId::MAX_NUM_NODES];
    uint32_t m_openConnections;
    std::vector<uint16_t> m_availableConnectionIds;

    std::unordered_map<uint32_t, uint16_t> m_qpNumToNodeIdMappings;

    Listener* m_listener;

    void* m_buffer;

private:
    struct NodeInfo
    {
        uint16_t m_nodeId;
        uint16_t m_lid;
        uint32_t m_physicalQpId[MAX_QPS_PER_CONNECTION];
    };

    struct PaketNodeInfo
    {
        uint32_t m_magic;
        NodeInfo m_info;
    } __attribute__((packed));
};

}
}

#endif // IBNET_CORE_IBCONNECTIONMANAGER_H
