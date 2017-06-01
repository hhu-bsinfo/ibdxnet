#include "IbMessageSystemTest.h"

#include <signal.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <argagg/argagg.hpp>
#include <backwards/backward.hpp>

#include "ibnet/sys/Logger.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/core/IbException.h"
#include "ibnet/core/IbNodeConfArgListReader.h"
#include "ibnet/msg/IbMessageSystem.h"

static std::shared_ptr<IbMessageSystemTest> g_messageSystemTest;

static void SignalHandler(int signal)
{
    g_messageSystemTest->SignalExit();
}

static ibnet::msg::Config GenerateConfig(argagg::parser_results& args)
{
    ibnet::msg::Config config;

    if (args["maxRecvReqs"]) {
        config.m_maxRecvReqs = args["maxRecvReqs"].as<uint16_t>();
    }
    if (args["maxSendReqs"]) {
        config.m_maxSendReqs = args["maxSendReqs"].as<uint16_t>();
    }
    if (args["inOutBufferSize"]) {
        config.m_inOutBufferSize = args["inOutBufferSize"].as<uint32_t>();
    }

    if (args["flowControlMaxRecvReqs"]) {
        config.m_flowControlMaxRecvReqs = args["flowControlMaxRecvReqs"].as<uint16_t>();
    }
    if (args["flowControlMaxSendReqs"]) {
        config.m_flowControlMaxSendReqs = args["flowControlMaxSendReqs"].as<uint16_t>();
    }

    if (args["connectionJobPoolSize"]) {
        config.m_connectionJobPoolSize = args["connectionJobPoolSize"].as<uint32_t>();
    }
    if (args["sendThreads"]) {
        config.m_sendThreads = args["sendThreads"].as<uint8_t>();
    }
    if (args["recvThreads"]) {
        config.m_recvThreads = args["recvThreads"].as<uint8_t>();
    }

    if (args["maxNumConnections"]) {
        config.m_maxNumConnections = args["maxNumConnections"].as<uint16_t>();
    }

    return config;
}

static ibnet::core::IbNodeConf GenerateNodeConfig(argagg::parser_results& args)
{
    ibnet::core::IbNodeConf nodeConf;

    for (auto& it : args.pos) {
        nodeConf.AddEntry(it);
    }

    return nodeConf;
}

int main(int argc, char** argv)
{
    backward::SignalHandling sh;

    ibnet::sys::Logger::Setup();

    signal(SIGINT, SignalHandler);

    argagg::parser argparser {{
          { "help", {"-h", "--help"},
              "shows this help message", 0},
          { "ownNodeId", {"-n", "--ownNodeId"},
              "Own node id", 1},
          { "remoteNodeId", {"-r", "--remoteNodeId"},
              "Remote node id", 1},
          { "msgSizeBytes", {"-m", "--msgSizeBytes"},
              "", 1},
          { "applicationThreadCount", {"-t", "--applicationThreadCount"},
              "", 1},
          { "maxRecvReqs", {"-a", "--maxRecvReqs"}, "", 1},
          { "maxSendReqs", {"-b", "--maxSendReqs"}, "", 1},
          { "inOutBufferSize", {"-c", "--inOutBufferSize"}, "", 1},
          { "flowControlMaxRecvReqs", {"-d", "--flowControlMaxRecvReqs"}, "", 1},
          { "flowControlMaxSendReqs", {"-e", "--flowControlMaxSendReqs"}, "", 1},
          { "connectionJobPoolSize", {"-f", "--connectionJobPoolSize"}, "", 1},
          { "sendThreads", {"-j", "--sendThreads"}, "", 1},
          { "recvThreads", {"-k", "--recvThreads"}, "", 1},
          { "maxNumConnections", {"-l", "--maxNumConnections"}, "", 1}
     }};

    std::ostringstream usage;
    usage
        << argv[0] << " 2.0" << std::endl
        << std::endl
        << "Usage: " << argv[0] << " [options]... -- <nodes>..." << std::endl
        << std::endl;

    argagg::parser_results args;
    try {
        args = argparser.parse(argc, argv);
    } catch (const std::exception& e) {
        std::cout << usage.str() << argparser << std::endl
             << "Encountered exception while parsing arguments: " << e.what()
             << std::endl;
        return -1;
    }

    if (args["help"]) {
        std::cout << usage.str() << argparser;
        return -1;
    }

    uint16_t ownNodeId;
    uint16_t remoteNodeId;
    uint32_t msgSizeBytes;
    uint32_t applicationThreadCount;

    if (!args["ownNodeId"] || !args["remoteNodeId"]) {
        std::cout << "Missing own node id and remote node id" << std::endl;
        return -1;
    }

    ownNodeId = args["ownNodeId"].as<uint16_t>();
    remoteNodeId = args["remoteNodeId"].as<uint16_t>();

    msgSizeBytes = args["msgSizeBytes"].as<uint32_t>(1024);
    applicationThreadCount = args["applicationThreadCount"].as<uint32_t>(1);

    ibnet::msg::Config config = GenerateConfig(args);
    ibnet::core::IbNodeConf nodeConfig = GenerateNodeConfig(args);

    g_messageSystemTest = std::make_shared<IbMessageSystemTest>(ownNodeId, nodeConfig, config);
    g_messageSystemTest->Execute(remoteNodeId, msgSizeBytes, applicationThreadCount);

    g_messageSystemTest.reset();

    return 0;
}


IbMessageSystemTest::IbMessageSystemTest(uint16_t ownNodeId,
        const ibnet::core::IbNodeConf& nodeConfig,
        const ibnet::msg::Config& config) :
    m_ownNodeId(ownNodeId),
    m_nodeConfig(nodeConfig),
    m_config(config),
    m_exit(false),
    m_system()
{
    memset(m_recvFcData, 0, sizeof(uint64_t) * ibnet::core::IbNodeId::MAX_NUM_NODES);
}

IbMessageSystemTest::~IbMessageSystemTest(void)
{

}

void IbMessageSystemTest::SignalExit(void)
{
    std::cout << "Signaled exit" << std::endl;
    m_exit.store(true);
}

void IbMessageSystemTest::Execute(uint16_t remoteNodeId, uint32_t msgSizeBytes,
        uint32_t applicationThreadCount)
{
    m_system = std::make_shared<ibnet::msg::IbMessageSystem>(m_ownNodeId,
        m_nodeConfig, m_config, shared_from_this(), nullptr, nullptr);

    if (m_ownNodeId == remoteNodeId) {
        std::cout << "Running server..." << std::endl;

        // just wait for incoming messages
        while (!m_exit.load()) {
            m_system->PrintStatus();
            __PrintReceiverStats();

            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    } else {
        std::cout << "Running client with " << applicationThreadCount << " application send threads" << std::endl;

        std::vector<std::unique_ptr<ApplicationSendThread>> m_senders;

        for (int i = 0; i < applicationThreadCount; i++) {
            auto t = std::make_unique<ApplicationSendThread>(remoteNodeId,
                msgSizeBytes, m_system);

            t->Start();
            m_senders.push_back(std::move(t));
        }

        while (!m_exit.load()) {
            __PrintReceiverStats();
            m_system->PrintStatus();
            for (auto& it : m_senders) {
                it->PrintStatistics();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }

        for (auto& it : m_senders) {
            it->Stop();
        }

        m_senders.clear();
    }

    m_system.reset();
}

void IbMessageSystemTest::HandleMessage(uint16_t source, void* buffer, uint32_t length)
{
    if (!m_recvThroughput[source].IsStarted()) {
        m_recvThroughput[source].Start();
    }

    m_recvThroughput[source].Update(length);

    while (!m_system->SendFlowControl(source, length)) {
        // queue full, wait a moment
        std::this_thread::yield();
    }
}

void IbMessageSystemTest::HandleFlowControlData(uint16_t source, uint32_t data)
{
    if (!m_recvFcThroughput[source].IsStarted()) {
        m_recvFcThroughput[source].Start();
    }

    m_recvFcData[source] += data;
    m_recvFcThroughput[source].Update(sizeof(uint32_t));
}

IbMessageSystemTest::ApplicationSendThread::ApplicationSendThread(
        uint16_t remoteNodeId, uint32_t msgBufferSize,
        std::shared_ptr<ibnet::msg::IbMessageSystem>& messageSystem) :
    m_remoteNodeId(remoteNodeId),
    m_msgBufferSize(msgBufferSize),
    m_messageSystem(messageSystem),
    m_buffer(malloc(msgBufferSize)),
    m_timer(),
    m_throughput()
{

}

IbMessageSystemTest::ApplicationSendThread::~ApplicationSendThread(void)
{
    free(m_buffer);
}

void IbMessageSystemTest::ApplicationSendThread::PrintStatistics(void)
{
    std::cout << m_throughput << std::endl;
}

void IbMessageSystemTest::ApplicationSendThread::_BeforeRunLoop(void)
{
    m_throughput.Start();
}

void IbMessageSystemTest::ApplicationSendThread::_RunLoop(void)
{
    try {
        m_timer.Enter();
        if (!m_messageSystem->SendMessage(m_remoteNodeId, m_buffer,
                m_msgBufferSize)) {
            // queue full, wait a moment
            std::this_thread::yield();
        } else {
            m_throughput.Update(m_msgBufferSize);
        }
        m_timer.Exit();
    } catch (ibnet::core::IbException &e) {
        m_timer.Exit();

        std::cout << "!!!!! " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void IbMessageSystemTest::ApplicationSendThread::_AfterRunLoop(void)
{
    m_throughput.Stop();

    std::cout << m_timer << std::endl << m_throughput << std::endl;
}

void IbMessageSystemTest::__PrintReceiverStats(void)
{
    for (uint32_t i = 0; i < ibnet::core::IbNodeId::MAX_NUM_NODES; i++) {
        if (m_recvThroughput[i].IsStarted()) {
            std::cout << "Node 0x" << std::hex << i << std::endl <<
                m_recvThroughput[i] << std::endl << m_recvFcThroughput[i] <<
                "recvFcData " << std::endl << std::dec << m_recvFcData[i] <<
                std::endl;
        }
    }
}