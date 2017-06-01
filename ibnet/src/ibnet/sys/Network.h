#ifndef IBNET_SYS_NETWORK_H
#define IBNET_SYS_NETWORK_H

#include <string>

#include "AddressIPV4.h"

namespace ibnet {
namespace sys {

class Network
{
public:
    static const std::string GetHostname(void);

    static AddressIPV4 ResolveHostname(const std::string& hostname);

    static AddressIPV4 ResolveIPForInterface(const std::string& iface);

private:
    Network(void) {};
    ~Network(void) {};
};

}
}

#endif //IBNET_SYS_NETWORK_H
