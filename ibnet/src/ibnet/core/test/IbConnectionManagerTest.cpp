#include <signal.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "ibnet/sys/Logger.h"

#include "ibnet/core/IbDiscoveryManager.h"
#include "ibnet/core/IbConnectionCreatorSimple.h"
#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbNodeConfArgListReader.h"

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
        printf("Usage: %s <node id> <remote node id> <hostnames nodes> ...\n", argv[0]);
        return 0;
    }

    uint16_t ownNodeId = atoi(argv[1]);
    uint16_t remoteNodeId = atoi(argv[2]);

    ibnet::sys::Logger::Setup();

    signal(SIGINT, SignalHandler);

    std::shared_ptr<ibnet::core::IbDevice> device =
        std::make_shared<ibnet::core::IbDevice>();

    std::shared_ptr<ibnet::core::IbProtDom> protDom =
        std::make_shared<ibnet::core::IbProtDom>(device, "default");

    uint32_t sizeBuffer = 1000;
    uint8_t* buffer = (uint8_t*) malloc(sizeBuffer);
    memset(buffer, 0, sizeBuffer);

    std::shared_ptr<ibnet::core::IbMemReg> memReg =
        protDom->Register(buffer, sizeBuffer, true);

    std::shared_ptr<ibnet::core::IbCompQueue> compQueue =
        std::make_shared<ibnet::core::IbCompQueue>(device, 100);

    ibnet::core::IbNodeConfArgListReader nodeConfArgListReader(
        (uint32_t) (argc - 3), &argv[3]);
    ibnet::core::IbNodeConf nodeConf = nodeConfArgListReader.Read();

    std::cout << "Own node id: 0x" <<  std::hex << ownNodeId << std::endl;
    std::shared_ptr<ibnet::core::IbDiscoveryManager> discMan =
        std::make_shared<ibnet::core::IbDiscoveryManager>(ownNodeId, nodeConf,
            5730, 1000);

    std::shared_ptr<ibnet::core::IbConnectionManager> conMan =
        std::make_shared<ibnet::core::IbConnectionManager>(ownNodeId, 5731, 100,
        device, protDom, discMan,
        std::make_unique<ibnet::core::IbConnectionCreatorSimple>(10, 10,
            std::shared_ptr<ibnet::core::IbSharedRecvQueue>(), compQueue));

    std::cout << "Running loop..." << std::endl;

    while (g_loop) {
        device->UpdateState();
        std::cout << *device << std::endl;
        std::cout << *conMan << std::endl;

        try {
            std::shared_ptr<ibnet::core::IbConnection> connection =
                conMan->GetConnection(remoteNodeId);
        } catch (const ibnet::core::IbException& e) {
            std::cout << "!!!!!" << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Cleanup..." << std::endl;

    conMan.reset();
    discMan.reset();
    compQueue.reset();
    protDom.reset();
    device.reset();

    return 0;
}