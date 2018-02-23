/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
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

#include <backwards/backward.hpp>

#include "ibnet/sys/Logger.hpp"
#include "ibnet/sys/Network.h"
#include "ibnet/sys/Random.h"

#include "ibnet/core/IbDevice.h"
#include "ibnet/core/IbProtDom.h"

#include "ibnet/con/NodeConfArgListReader.h"

#include "DummyConnection.h"
#include "DummyConnectionManager.h"

class ClientThread : public ibnet::sys::ThreadLoop
{
public:
    ClientThread(uint32_t id, ibnet::con::NodeId ownNodeID,
        const std::vector<std::string>& hostnamesSorted,
        ibnet::con::ConnectionManager* refConMan) :
        ThreadLoop("ClientThread-" + std::to_string(id)),
        m_id(id),
        m_ownNodeId(ownNodeID),
        m_hostnamesSorted(hostnamesSorted),
        m_refConnectionManager(refConMan)
    {};

    ~ClientThread() override = default;

protected:
    void _RunLoop() override
    {
        // -1: don't count own node
        uint16_t notConnected =
            static_cast<uint16_t>(m_hostnamesSorted.size() - 1);
        uint16_t notConnectedPrev = notConnected;

        for (uint16_t i = 0; i < m_hostnamesSorted.size(); i++) {
            ibnet::con::NodeId remoteNodeId = i;

            if (remoteNodeId == m_ownNodeId) {
                continue;
            }

            try {
                ibnet::con::Connection* connection =
                    m_refConnectionManager->GetConnection(remoteNodeId);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                m_refConnectionManager->ReturnConnection(connection);
                notConnected--;
            } catch (const ibnet::sys::Exception& e) {
                printf("!!! Exception: %s\n", e.what());
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if (m_id == 0 && notConnectedPrev > notConnected) {
            if (notConnected <= 0) {
                printf("***** ALL CONNECTED *****\n");
            } else {
                printf("Waiting for %d more node(s) to connect...\n",
                    notConnected);
            }
        }

        if (notConnected == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    const uint32_t m_id;
    const ibnet::con::NodeId m_ownNodeId;
    std::vector<std::string> m_hostnamesSorted;
    ibnet::con::ConnectionManager* m_refConnectionManager;
};

class Listener : public ibnet::con::DiscoveryListener,
    public ibnet::con::ConnectionListener
{
public:
    Listener() = default;

    ~Listener() override = default;

    void NodeDiscovered(ibnet::con::NodeId nodeId) override
    {
        printf("Node discovered: 0x%X\n", nodeId);
    }

    void NodeInvalidated(ibnet::con::NodeId nodeId) override
    {
        printf("Nov invalidated: 0x%X\n", nodeId);
    }

    void NodeConnected(ibnet::con::Connection& connection)
    {
        printf("Node connected: 0x%X\n", connection.GetRemoteNodeId());
    }

    void NodeDisconnected(ibnet::con::NodeId nodeId)
    {
        printf("Node disconnected: 0x%X\n", nodeId);
    }
};

static bool g_loop = true;

static void SignalHandler(int signal)
{
    if (signal == SIGINT) {
        g_loop = false;
    }
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Usage: %s <num client threads> <hostnames nodes> ...\n", argv[0]);
        return -1;
    }

    // bootstrapping

    backward::SignalHandling sh;

    ibnet::sys::Random::Init();
    ibnet::sys::Logger::Setup();

    signal(SIGINT, SignalHandler);

    // cmd args processing

    auto clientThreads = static_cast<uint32_t>(std::atol(argv[1]));

    std::vector<std::string> hostnamesSorted;
    ibnet::con::NodeConfArgListReader nodeConfArgListReader(
        (uint32_t) (argc - 2), &argv[2]);
    ibnet::con::NodeConf nodeConf = nodeConfArgListReader.Read();

    for (int i = 2; i < argc; i++) {
        hostnamesSorted.push_back(std::string(argv[i]));
    }

    std::sort(hostnamesSorted.begin(), hostnamesSorted.end());

    ibnet::con::NodeId ownNodeId = ibnet::con::NODE_ID_INVALID;
    const std::string ownHostname = ibnet::sys::Network::GetHostname();

    // auto assign a node id by using the sorted hostname list

    uint16_t counter = 0;
    for (auto& it : hostnamesSorted) {
        if (ownHostname == it) {
            ownNodeId = counter;
            break;
        }

        counter++;
    }

    if (ownNodeId == ibnet::con::NODE_ID_INVALID) {
        printf("ERROR Could not assign node id to current host %s , not found "
            "in hostname nodes list\n", ownHostname.c_str());
        return -1;
    }

    printf("Own node id: 0x%X\n", ownNodeId);

    // ib setup

    auto* device = new ibnet::core::IbDevice();
    auto* protDom = new ibnet::core::IbProtDom(*device, "ConTest");

    // connection manager

    auto* listener = new Listener();

    auto* exchangeManager = new ibnet::con::ExchangeManager(
        ownNodeId, 5730);
    auto* jobManager = new ibnet::con::JobManager();

    auto* discoveryManager = new ibnet::con::DiscoveryManager(
        ownNodeId, nodeConf, exchangeManager, jobManager);
    discoveryManager->SetListener(listener);

    auto* connectionManager = new ibnet::con::DummyConnectionManager(ownNodeId,
        nodeConf, 5000, 100, device, protDom, exchangeManager, jobManager,
        discoveryManager);
    connectionManager->SetListener(listener);

    // run client threads creating connections

    std::vector<ClientThread*> threads;

    for (uint32_t i = 0; i < clientThreads; i++) {
        auto thread = new ClientThread(i, ownNodeId,
            hostnamesSorted, connectionManager);

        thread->Start();
        threads.push_back(std::move(thread));
    }

    printf("Running %d client threads...\n", clientThreads);

    while (g_loop) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    printf("Cleanup...\n");

    for (auto& it : threads) {
        it->Stop();
        delete it;
    }

    threads.clear();

    delete connectionManager;
    delete discoveryManager;
    delete jobManager;
    delete exchangeManager;

    delete listener;

    delete protDom;
    delete device;

    ibnet::sys::Logger::Shutdown();

    return 0;
}