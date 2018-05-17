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

#include "MsgrcLoopbackSystem.h"

#include <argagg/argagg.hpp>

#include "ibnet/con/InvalidNodeIdException.h"
#include "ibnet/con/NodeConfStringReader.h"

#include "ibnet/msgrc/RecvHandler.h"

namespace ibnet {
namespace msgrc {

MsgrcLoopbackSystem::MsgrcLoopbackSystem(int argc, char** argv) :
        MsgrcSystem(),
        m_sendTargetNodeIds(),
        m_availableTargetNodes(),
        m_targetNodesAvailable(0),
        m_workPackage(),
        m_nodeToSendToPos(0)
{
    _SetConfiguration(__ProcessCmdArgs(argc, argv));

    for (bool& it : m_availableTargetNodes) {
        it = false;
    }
}

void MsgrcLoopbackSystem::NodeDiscovered(con::NodeId nodeId)
{
    IBNET_LOG_DEBUG("Node discovered: 0x%X", nodeId);

    m_availableTargetNodes[nodeId] = true;
    m_targetNodesAvailable.fetch_add(1, std::memory_order_release);
}

void MsgrcLoopbackSystem::NodeInvalidated(con::NodeId nodeId)
{
    IBNET_LOG_DEBUG("Node invalidated: 0x%X", nodeId);
}

void MsgrcLoopbackSystem::NodeConnected(con::Connection& connection)
{
    IBNET_LOG_DEBUG("Node connected: 0x%X", connection.GetRemoteNodeId());
}

void MsgrcLoopbackSystem::NodeDisconnected(con::NodeId nodeId)
{
    IBNET_LOG_DEBUG("Node disconnected: 0x%X", nodeId);

    m_targetNodesAvailable.fetch_sub(1);
    m_availableTargetNodes[nodeId] = false;
}

uint32_t MsgrcLoopbackSystem::Received(const IncomingRingBuffer::RingBuffer* ringBuffer)
{
    // just return buffers back to pool
    for (uint32_t i = 0; i < ringBuffer->m_usedEntries; i++) {
        m_recvBufferPool->ReturnBuffer(ringBuffer->m_entries[(ringBuffer->m_front + i) % ringBuffer->m_size].m_data);
    }

    return ringBuffer->m_usedEntries;
}

const SendHandler::NextWorkPackage* MsgrcLoopbackSystem::GetNextDataToSend(
        const PrevWorkPackageResults* prevResults,
        const SendHandler::CompletedWorkList* completionList)
{
    // wait for all send target nodes to be available
    if (m_sendTargetNodeIds.size() > 0 &&
            m_targetNodesAvailable.load(std::memory_order_acquire) >=
                    m_sendTargetNodeIds.size()) {

        if (m_availableTargetNodes[m_sendTargetNodeIds[m_nodeToSendToPos]]) {
            // always send full send buffer
            m_workPackage.m_posBackRel = 0;
            m_workPackage.m_posFrontRel = m_configuration->m_sendBufferSize;
            m_workPackage.m_flowControlData = 0;
            m_workPackage.m_nodeId = m_sendTargetNodeIds[m_nodeToSendToPos];
        } else {
            // node not discovered, don't send anything
            m_workPackage.m_posBackRel = 0;
            m_workPackage.m_posFrontRel = 0;
            m_workPackage.m_flowControlData = 0;
            m_workPackage.m_nodeId = con::NODE_ID_INVALID;
        }

        m_nodeToSendToPos++;

        if (m_nodeToSendToPos >= m_sendTargetNodeIds.size()) {
            m_nodeToSendToPos = 0;
        }
    } else {
        m_workPackage.m_posBackRel = 0;
        m_workPackage.m_posFrontRel = 0;
        m_workPackage.m_flowControlData = 0;
        m_workPackage.m_nodeId = con::NODE_ID_INVALID;
    }

    return &m_workPackage;
}

void MsgrcLoopbackSystem::_PostInit()
{
    std::string str;

    for (auto& it : m_sendTargetNodeIds) {
        str += std::to_string(it) + ", ";
    }

    IBNET_LOG_INFO("Send target node ids: %s", str);
}

MsgrcSystem::Configuration* MsgrcLoopbackSystem::__ProcessCmdArgs(
        int argc, char** argv)
{
    auto* config = new Configuration();

    argagg::parser argparser = {{
            {
                    "help",
                    {"-h", "--help"},
                    "shows this help message",
                    0
            },
            {
                    "ownNodeId",
                    {"-n", "--ownNodeId"},
                    "set the node id of this instance (required option)",
                    1
            },
            {
                    "nodeConfig",
                    {"-c", "--nodeConfig"},
                    "A list of hostnames of nodes that are part of the network (either "
                            "for sending or receiving data), e.g. node65,node66,node67",
                    1
            },
            {
                    "targetNodeIds",
                    {"-d", "--targetNodeIds"},
                    "A list of target node ids to send data to, e.g. 1,2,5. Leave"
                            "empty if node is on receive, only",
                    1
            },
            {
                    "pinSendRecvThreads",
                    {"-j", "--pinSendRecvThreads"},
                    "pin the send and recv thread to a single cpu core each",
                    0
            },
            {
                    "enableSignalHandler",
                    {"-w", "--enableSignalHandler"},
                    "enable a custom signal handler (for debugging)",
                    0
            },
            {
                    "statisticsThreadPrintIntervalMs",
                    {"-u", "--statisticsThreadPrintIntervalMs"},
                    "print recorded performance statistics every X ms to the console "
                            "(for debugging). 0 to disable.",
                    1
            },
            {
                    "portDiscMan",
                    {"-p", "--portDiscMan"},
                    "set the UDP port for the DiscoveryManager to use (must be "
                            "identical with other instances)",
                    1
            },
            {
                    "connectionCreationTimeoutMs",
                    {"-t", "--connectionCreationTimeoutMs"},
                    "Amount of time to wait for a new connection to be established "
                            "(ms)",
                    1
            },
            {
                    "maxNumConnections",
                    {"-m", "--maxNumConnections"},
                    "Max number of simultaneous opened connections allowed",
                    1
            },
            {
                    "sqSize",
                    {"-s", "--sqSize"},
                    "Max number of send work requests per send queue per connection",
                    1
            },
            {
                    "srqSize",
                    {"-r", "--srqSize"},
                    "Max number of receive work requests for all connections (shared "
                            "receive queue)",
                    1
            },
            {
                    "sharedSCQSize",
                    {"-a", "--sharedSCQSize"},
                    "Max number of work completions for shared send completion queue",
                    1
            },
            {
                    "sharedRCQSize",
                    {"-b", "--sharedRCQSize"},
                    "Max number of work completions for shared recv completion queue",
                    1
            },
            {
                    "sendBufferSize",
                    {"-e", "--sendBufferSize"},
                    "Max size of the send (ring) buffer per connection (in bytes)",
                    1
            },
            {
                    "recvBufferPoolSize",
                    {"-f", "--recvBufferPoolSize"},
                    "Total size of the receive buffer pool (in bytes)",
                    1
            },
            {
                    "recvBufferSize",
                    {"-g", "--recvBufferSize"},
                    "Size of a single receive buffer (in bytes)",
                    1
            },
            {
                    "maxSge",
                    {"-i", "--maxSge"},
                    "Max number of SGEs to use for WRQs (for receiving)",
                    1
            },
    }};

    argagg::parser_results args = argparser.parse(argc, argv);

    if (args["help"]) {
        printf("MsgRC MsgrcLoopbackSystem Test to test and measure performance "
                "of msgrc dispatchers\n");
        printf("Usage: %s [Options...]\n", argv[0]);
        printf("Available options:\n");

        // because std::cout << argparser << std::endl; doesn't compile with
        // some versions of gcc
        for (auto& definition : argparser.definitions) {
            std::cout << "    ";
            for (auto& flag : definition.flags) {
                std::cout << flag;
                if (flag != definition.flags.back()) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
            std::cout << "        " << definition.help << std::endl;
        }

        std::cout << std::endl;
        throw sys::Exception("Help called");
    }

    if (args["ownNodeId"]) {
        config->m_ownNodeId =
                args["ownNodeId"].as<ibnet::con::NodeId>(config->m_ownNodeId);
    }

    if (args["nodeConfig"]) {
        con::NodeConfStringReader reader(
                args["nodeConfig"].as<std::string>(""));
        config->m_nodeConfig = reader.Read();
    }

    if (args["targetNodeIds"]) {
        std::vector<std::string> tokens = sys::StringUtils::Split(
                args["targetNodeIds"].as<std::string>(""), ",");

        for (auto& it : tokens) {
            m_sendTargetNodeIds.push_back(
                    static_cast<ibnet::con::NodeId>(std::atoi(it.c_str())));
        }
    }

    if (args["enableSignalHandler"]) {
        config->m_enableSignalHandler =
                args["enableSignalHandler"].as<bool>(config->m_enableSignalHandler);
    }

    if (args["pinSendRecvThreads"]) {
        config->m_pinSendRecvThreads =
                args["pinSendRecvThreads"].as<bool>(config->m_pinSendRecvThreads);
    }

    if (args["statisticsThreadPrintIntervalMs"]) {
        config->m_statisticsThreadPrintIntervalMs =
                args["statisticsThreadPrintIntervalMs"].as<uint32_t>(
                        config->m_statisticsThreadPrintIntervalMs);
    }

    if (args["portDiscMan"]) {
        config->m_portDiscMan =
                args["portDiscMan"].as<uint16_t>(config->m_portDiscMan);
    }

    if (args["connectionCreationTimeoutMs"]) {
        config->m_connectionCreationTimeoutMs =
                args["connectionCreationTimeoutMs"].as<uint32_t>(
                        config->m_connectionCreationTimeoutMs);
    }

    if (args["maxNumConnections"]) {
        config->m_maxNumConnections =
                args["maxNumConnections"].as<uint16_t>(config->m_maxNumConnections);
    }

    if (args["sqSize"]) {
        config->m_SQSize =
                args["sqSize"].as<uint16_t>(config->m_SQSize);
    }

    if (args["srqSize"]) {
        config->m_SRQSize =
                args["srqSize"].as<uint16_t>(config->m_SRQSize);
    }

    if (args["sharedSCQSize"]) {
        config->m_sharedSCQSize =
                args["sharedSCQSize"].as<uint16_t>(config->m_sharedSCQSize);
    }

    if (args["sharedRCQSize"]) {
        config->m_sharedRCQSize =
                args["sharedRCQSize"].as<uint16_t>(config->m_sharedRCQSize);
    }

    if (args["sendBufferSize"]) {
        config->m_sendBufferSize =
                args["sendBufferSize"].as<uint32_t>(config->m_sendBufferSize);
    }

    if (args["recvBufferPoolSize"]) {
        config->m_recvBufferPoolSizeBytes =
                args["recvBufferPoolSize"].as<uint64_t>(
                        config->m_recvBufferPoolSizeBytes);
    }

    if (args["recvBufferSize"]) {
        config->m_recvBufferSize =
                args["recvBufferSize"].as<uint32_t>(config->m_recvBufferSize);
    }

    if (args["maxSge"]) {
        config->m_maxSGEs = args["maxSge"].as<uint16_t>(config->m_maxSGEs);
    }

    if (config->m_ownNodeId == con::NODE_ID_INVALID) {
        throw con::InvalidNodeIdException(config->m_ownNodeId,
                "Provide a valid one via cmd args");
    }

    return config;
}

}
}
