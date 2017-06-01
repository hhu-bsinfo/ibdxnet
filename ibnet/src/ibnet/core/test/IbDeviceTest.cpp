#include <signal.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "ibnet/sys/Logger.h"
#include "ibnet/core/IbDevice.h"

static bool g_loop = true;

static void SignalHandler(int signal)
{
    if (signal == SIGINT) {
        g_loop = false;
    }
}

int main(int argc, char** argv)
{
    ibnet::sys::Logger::Setup();

    signal(SIGINT, SignalHandler);

    ibnet::core::IbDevice device;

    std::cout << "Running loop..." << std::endl;

    while (g_loop) {
        device.UpdateState();
        std::cout << device << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Cleanup..." << std::endl;

    return 0;
}