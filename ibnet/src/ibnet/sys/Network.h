#ifndef IBNET_SYS_NETWORK_H
#define IBNET_SYS_NETWORK_H

#include <string>

#include "AddressIPV4.h"

namespace ibnet {
namespace sys {

/**
 * Helper class for network related tasks
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Network
{
public:
    /**
     * Get the currently set hostname of the system
     */
    static const std::string GetHostname(void);

    /**
     * Resolve a hostname to an IPV4 address
     *
     * @param hostname Hostname to resolve
     * @return If successful, a valid AddressIPV4 object, otherwise the
     *          returned object is set invalid.
     */
    static AddressIPV4 ResolveHostname(const std::string& hostname);

    /**
     * Resolve the name of a network interface to an IPV4 address
     *
     * @param iface Interface name to resolve
     * @return If successful, a valid AddressIPV4 object, otherwise the
     *          returned object is set invalid.
     */
    static AddressIPV4 ResolveIPForInterface(const std::string& iface);

private:
    Network(void) {};
    ~Network(void) {};
};

}
}

#endif //IBNET_SYS_NETWORK_H
