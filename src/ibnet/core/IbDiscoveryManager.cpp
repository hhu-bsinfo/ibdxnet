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

#include "IbDiscoveryManager.h"

#include <spdlog/fmt/ostr.h>

#include "ibnet/sys/Network.h"
#include "ibnet/sys/Logger.hpp"

#include "IbNodeNotAvailableException.h"

#define PAKET_MAGIC 0xDEADBEEF

namespace ibnet {
namespace core {

IbDiscoveryManager::IbDiscoveryManager(
        uint16_t ownNodeId,
        const IbNodeConf& nodeConf,
        uint16_t udpPort,
        uint32_t discoveryIntervalMs) :
    ThreadLoop("DiscoveryManager"),
    m_ownNodeId(ownNodeId),
    m_infoToGet(),
    m_socketPort(udpPort),
    m_socket(std::make_unique<sys::SocketUDP>(m_socketPort)),
    m_nodeInfo(),
    m_listener(nullptr),
    m_discoveryIntervalMs(discoveryIntervalMs),
    m_activePhase(false)
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("Initializing node discovery manager, own node id 0x{:x}...",
            ownNodeId);

    std::string ownHostname = sys::Network::GetHostname();

    for (auto& it : nodeConf.GetEntries()) {

        // don't add ourselves
        if (it.GetHostname() != ownHostname) {
            m_infoToGet.push_back(std::make_shared<IbNodeConf::Entry>(it));
        }
    }

    Start();

    IBNET_LOG_DEBUG("Initializing node discovery manager done");
}

IbDiscoveryManager::~IbDiscoveryManager(void)
{
    IBNET_LOG_TRACE_FUNC;
    IBNET_LOG_INFO("Shutting down node discovery manager...");

    Stop();

    m_socket.reset();

    IBNET_LOG_DEBUG("Shutting down node discovery manager done");
}

void IbDiscoveryManager::AddNode(const IbNodeConf::Entry& entry)
{
    IBNET_LOG_TRACE_FUNC;

    std::string ownHostname = sys::Network::GetHostname();

    // don't add ourselves
    if (entry.GetHostname() != ownHostname) {
        IBNET_LOG_INFO("Adding node {}", entry);

        std::lock_guard<std::mutex> l(m_lock);
        m_infoToGet.push_back(std::make_shared<IbNodeConf::Entry>(entry));
    }
}

const std::shared_ptr<IbNodeConf::Entry>& IbDiscoveryManager::GetNodeInfo(
        uint16_t nodeId)
{
    IBNET_LOG_TRACE_FUNC;

    std::lock_guard<std::mutex> l(m_lock);

    if (!m_nodeInfo[nodeId]) {
        throw IbNodeNotAvailableException(nodeId);
    }

    return m_nodeInfo[nodeId];
}

void IbDiscoveryManager::InvalidateNodeInfo(uint16_t nodeId)
{
    m_lock.lock();
    m_infoToGet.push_back(m_nodeInfo[nodeId]);
    m_nodeInfo[nodeId].reset();
    m_lock.unlock();

    if (m_listener) {
        m_listener->NodeInvalidated(nodeId);
    }
}

void IbDiscoveryManager::_RunLoop(void)
{
    PaketNodeInfo ownNodeInfo;
    ownNodeInfo.m_magic = PAKET_MAGIC;
    ownNodeInfo.m_paketId = e_PaketIdInfo;
    ownNodeInfo.m_nodeId = m_ownNodeId;

    PaketNodeInfo paketBuffer;
    const size_t length = sizeof(PaketNodeInfo);

    paketBuffer.m_magic = PAKET_MAGIC;
    paketBuffer.m_paketId = e_PaketIdReq;

    m_lock.lock();

    IBNET_LOG_TRACE("Requesting node info of {} nodes", m_infoToGet.size());

    // request remote node's information if not received, yet
    for (auto& it : m_infoToGet) {
        IBNET_LOG_TRACE("Requesting node info from {}:{}",
                it->GetAddress().GetAddressStr(), m_socketPort);
        ssize_t res = m_socket->Send(&paketBuffer, length,
                it->GetAddress().GetAddress(), m_socketPort);

        if (res != length) {
            IBNET_LOG_ERROR("Sending node info req to {} failed",
                    it->GetAddress().GetAddressStr());
        }
    }

    m_lock.unlock();

    uint32_t recvAddr = 0;

    // listen for incoming responses, try this a few times
    for (uint32_t i = 0; i < 5; i++) {
        ssize_t res = m_socket->Receive(&paketBuffer, length, &recvAddr);

        if (res > 0) {
            m_activePhase = true;

            IBNET_LOG_TRACE("Received data from {}",
                    sys::AddressIPV4(recvAddr));

            if (paketBuffer.m_magic != PAKET_MAGIC) {
                IBNET_LOG_ERROR("Received paket with invalid magic 0x{:x}",
                        paketBuffer.m_magic);
                continue;
            }

            if (paketBuffer.m_paketId == e_PaketIdReq) {
                IBNET_LOG_TRACE("Received node info REQ from {}, answering",
                        sys::AddressIPV4(recvAddr));

                // reply with own node information
                m_socket->Send(&ownNodeInfo, sizeof(ownNodeInfo),
                        recvAddr, m_socketPort);
            } else if (paketBuffer.m_paketId == e_PaketIdInfo) {
                bool removed = false;

                IBNET_LOG_TRACE("Received node info RESP from {}",
                        sys::AddressIPV4(recvAddr));

                m_lock.lock();

                // remove from processing list
                for (auto it = m_infoToGet.begin();
                     it != m_infoToGet.end(); it++) {
                    if ((*it)->GetAddress().GetAddress() == recvAddr) {
                        IBNET_LOG_INFO("Discovered node {} as node id 0x{:x}",
                                (*it)->GetAddress().GetAddressStr(),
                                paketBuffer.m_nodeId);

                        // store remote node information
                        m_nodeInfo[paketBuffer.m_nodeId] = *it;

                        it = m_infoToGet.erase(it);
                        removed = true;

                        break;
                    }
                }

                m_lock.unlock();

                if (removed && m_listener) {
                    m_listener->NodeDiscovered(paketBuffer.m_nodeId);
                }
            } else {
                IBNET_LOG_ERROR("Received paket with unknown id {}",
                        paketBuffer.m_paketId);
                continue;
            }
        }
    }

    if (m_activePhase) {
        m_activePhase = false;
        // don't sleep, keep going until there is nothing left to be processed
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(
            m_discoveryIntervalMs));
    }
}

}
}