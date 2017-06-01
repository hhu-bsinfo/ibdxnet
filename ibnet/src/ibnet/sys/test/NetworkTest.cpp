#include <cstdio>

#include "ibnet/sys/Logger.h"
#include "ibnet/sys/Network.h"

int main(int argc, char** argv)
{
    ibnet::sys::Logger::Setup();

    std::string hostname = ibnet::sys::Network::GetHostname();
    std::printf("Own hostname: %s\n", hostname.c_str());
    std::printf("Own hostname ip: %s\n",
            ibnet::sys::Network::ResolveHostname(hostname).GetAddressStr().c_str());
    std::printf("IP for eth0: %s",
            ibnet::sys::Network::ResolveIPForInterface("eth0").GetAddressStr().c_str());

    return 0;
}