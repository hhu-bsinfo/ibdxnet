#ifndef IBNET_SYS_SOCKETUDP_H
#define IBNET_SYS_SOCKETUDP_H

#include <stdio.h>

#include <cstdint>

#include "AddressIPV4.h"

namespace ibnet {
namespace sys {

class SocketUDP
{
public:
    SocketUDP(uint16_t port);

    ~SocketUDP(void);

    uint16_t GetPort(void) const {
        return m_port;
    }

    ssize_t Receive(void* buffer, size_t size, uint32_t* recvIpv4);

    ssize_t Send(void* buffer, size_t size, uint32_t addrIpv4, uint16_t port);

private:
    uint16_t m_port;
    int m_socket;
};

}
}

#endif //IBNET_SYS_SOCKETUDP_H
