#ifndef IBNET_SYS_SOCKETUDP_H
#define IBNET_SYS_SOCKETUDP_H

#include <stdio.h>

#include <cstdint>

#include "AddressIPV4.h"

namespace ibnet {
namespace sys {

/**
 * UDP (unreliable) based network socket
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class SocketUDP
{
public:
    /**
     * Constructor
     *
     * Open the socket on the defined port in non-blocking mode
     *
     * @param port Port to open the socket on
     */
    SocketUDP(uint16_t port);

    /**
     * Destructor
     */
    ~SocketUDP(void);

    /**
     * Get the port of this socket
     */
    uint16_t GetPort(void) const {
        return m_port;
    }

    /**
     * Receive data (non-blocking!)
     *
     * @param buffer Allocated buffer to write the received data to
     * @param size Size of the allocated buffer
     * @param recvIpv4 Pointer to variable to return the IPV4 of the sender of
     *          the received data to
     * @return Number of bytes actually received and written to the buffer or
     *          -1 on error. If 0, no data was available.
     */
    ssize_t Receive(void* buffer, size_t size, uint32_t* recvIpv4);

    /**
     * Send data (non-blocking!)
     *
     * @param buffer Allocated buffer with data to send
     * @param size Number of bytes to send
     * @param addrIpv4 Destination IPV4 to send to
     * @param port  Destination port
     * @return Number of bytes sent
     */
    ssize_t Send(void* buffer, size_t size, uint32_t addrIpv4, uint16_t port);

private:
    uint16_t m_port;
    int m_socket;
};

}
}

#endif //IBNET_SYS_SOCKETUDP_H
