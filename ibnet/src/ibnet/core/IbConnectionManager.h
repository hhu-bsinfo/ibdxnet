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

/**
 * The connection manager is the core which manages automatic node discovery
 * (using the DiscoveryManager), connection creation, setup, management as well
 * as handling lost connections and cleanup.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
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

    /**
     * Constructor
     *
     * @param ownNodeId Own node id
     * @param socketPort Port for the (ethernet) socket for the discovery manager
     * @param maxNumConnections Max number of simultanious connections to be
     *          handled by the manager
     * @param device An opened device
     * @param protDom Protection domain bound to the device
     * @param discoveryManager DiscoveryManager to use for connection creation
     * @param connectionCreator ConnectionCreator implementation which
     *          determines how to setup newly established connections
     */
    IbConnectionManager(
            uint16_t ownNodeId,
            uint16_t socketPort,
            uint32_t maxNumConnections,
            std::shared_ptr<IbDevice>& device,
            std::shared_ptr<IbProtDom>& protDom,
            std::shared_ptr<IbDiscoveryManager>& discoveryManager,
            std::unique_ptr<IbConnectionCreator> connectionCreator);

    /**
     * Destructor
     */
    ~IbConnectionManager(void);

    /**
     * Set a connection listener which listens to node connect/disconnect
     * events
     *
     * @param listener Listener to set
     */
    void SetNodeConnectedListener(Listener* listener) {
        m_listener = listener;
    }

    /**
     * Get the (remote) node id of a node with a physical QP num
     *
     * @param qpNum QP num of the node to get the node id of
     * @return Node id of the node owning the QP with the physical QP id
     */
    uint16_t GetNodeIdForPhysicalQPNum(uint32_t qpNum) {
        auto it = m_qpNumToNodeIdMappings.find(qpNum);

        if (it != m_qpNumToNodeIdMappings.end()) {
            return it->second;
        }

        return IbNodeId::INVALID;
    }

    /**
     * Check if a connection is available (open and connected)
     *
     * @param destination Remote node id
     * @return True if available, false otherwise
     */
    bool IsConnectionAvailable(uint16_t nodeId);

    /**
     * Get a connection
     *
     * If called for the first time with a node id, this might establish the
     * connection to the specified node id but keeps it opened either until
     * closed or the max number of connections is exceeded and the connection
     * must be suppressed.
     *
     * Use the returned shared pointer as a handle to determine who is still
     * owning a reference to the connection.
     *
     * Do not keep/store the shared pointer. If you want to operate on a
     * connection, always call this function to get the connection.
     *
     * @param nodeId Get the connection of this node
     * @return If successful, a valid pointer to the established connection.
     *          Throws exceptions on errors.
     */
    std::shared_ptr<IbConnection> GetConnection(uint16_t nodeId);

    // TODO doc
    void ReturnConnection(std::shared_ptr<IbConnection>& connection);

    /**
     * Explicitly close a connection
     *
     * @param nodeId Node id of the connection to close
     * @param force True to force close (don't wait for queues to be emptied),
     *          false to empty the queues and ensure everything queued is still
     *          being processed/
     */
    void CloseConnection(uint16_t nodeId, bool force);

    /**
     * Enable output to an out stream
     */
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
    static const int32_t CONNECTION_NOT_AVAILABLE = INT32_MIN;
    static const int32_t CONNECTION_AVAILABLE = 0;
    static const int32_t CONNECTION_CLOSING = INT32_MIN / 2;

private:
    sys::SocketUDP m_socket;

    uint32_t m_maxNumConnections;

    std::shared_ptr<IbDevice> m_device;
    std::shared_ptr<IbProtDom> m_protDom;
    std::shared_ptr<IbDiscoveryManager> m_discoveryManager;
    std::unique_ptr<IbConnectionCreator> m_connectionCreator;

    uint16_t m_ownNodeId;

    std::mutex m_connectionMutex;
    std::atomic<int32_t> m_connectionAvailable[IbNodeId::MAX_NUM_NODES];
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
