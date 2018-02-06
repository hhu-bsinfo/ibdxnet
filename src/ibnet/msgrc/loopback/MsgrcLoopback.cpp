#include <chrono>
#include <thread>

#include "MsgrcLoopbackSystem.h"

int main(int argc, char** argv)
{
    auto* loopback = new ibnet::msgrc::MsgrcLoopbackSystem(argc, argv);

    loopback->Init();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    loopback->Shutdown();

    delete loopback;
    return 0;
}