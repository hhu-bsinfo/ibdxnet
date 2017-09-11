/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

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

/**
 * The discovery manager requires a standard ethernet connection of all nodes
 * with InfiniBand hardware. To connect to other nodes using InfiniBand,
 * connection information needs to be exchanged prior queue pair creation.
 * Obviously, this is not possible via InfiniBand. Thus, the discovery
 * manager is handling this exchange over ethernet.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
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

    /**
     * Constructor
     *
     * @param ownNodeId Node id of the current node
     * @param nodeConf Node config to use for discovering further nodes
     * @param udpPort Port to run the UDP socket on
     */
    IbDiscoveryManager(uint16_t ownNodeId, const IbNodeConf& nodeConf,
        uint16_t udpPort);

    /**
     * Destructor
     */
    ~IbDiscoveryManager(void);

    /**
     * Set a listener which receives callbacks on newly discovered or
     * invalidated nodes
     *
     * @param listener Pointer to listener to set
     */
    void SetNodeDiscoveryListener(Listener* listener) {
        m_listener = listener;
    }

    /**
     * Add another node to the manager to allow discovery
     *
     * @param entry NodeConf entry to add to the existing config used by the
     *          manager
     */
    void AddNode(const IbNodeConf::Entry& entry);

    /**
     * Get the node information used for discovery
     *
     * @param nodeId Node id of the node
     * @return If found entry with information used for discovery,
     *          null otherwise
     */
    const std::shared_ptr<IbNodeConf::Entry>& GetNodeInfo(uint16_t nodeId);

    /**
     * Invalidate node info. Call this if you detected that the connection
     * was lost. This adds the node to the "to discover" nodes list
     *
     * @param nodeId Id of node with lost connection
     */
    void InvalidateNodeInfo(uint16_t nodeId);

protected:
    void _RunLoop(void) override;

private:
    const uint32_t m_discoveryIntervalMs;

    uint16_t m_ownNodeId;
    std::vector<std::shared_ptr<IbNodeConf::Entry>> m_infoToGet;

    uint16_t m_socketPort;
    std::unique_ptr<sys::SocketUDP> m_socket;

    std::mutex m_lock;
    std::shared_ptr<IbNodeConf::Entry> m_nodeInfo[IbNodeId::MAX_NUM_NODES];

    Listener* m_listener;

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
