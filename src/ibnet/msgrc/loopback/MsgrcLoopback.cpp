#include <chrono>
#include <thread>

#include "MsgrcLoopbackSystem.h"

static bool g_loop = true;

static void SignalHandler(int signal)
{
    if (signal == SIGINT) {
        g_loop = false;
    }
}

int main(int argc, char** argv)
{
    auto* loopback = new ibnet::msgrc::MsgrcLoopbackSystem(argc, argv);

    loopback->Init();

    signal(SIGINT, SignalHandler);

    while (g_loop) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    loopback->Shutdown();

    delete loopback;
    return 0;
}