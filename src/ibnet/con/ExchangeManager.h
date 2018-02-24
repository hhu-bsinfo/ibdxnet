/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_CON_EXCHANGEMANAGER_H
#define IBNET_CON_EXCHANGEMANAGER_H

#include "ibnet/sys/SocketUDP.h"
#include "ibnet/sys/ThreadLoop.h"

#include "NodeConf.h"
#include "NodeId.h"

namespace ibnet {
namespace con {

// forward declaration
class ExchangeDispatcher;

/**
 * This class is providing a side channel (UDP socket) to exchange data
 * (e.g. connection data) with a remote node.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
class ExchangeManager : public sys::ThreadLoop
{
public:
    typedef uint8_t PaketType;
    static const uint8_t INVALID_PACKET_TYPE = 0xFF;

    static const size_t MAX_PAKET_SIZE = 1024 * 64;

public:
    /**
     * Paket header for sending/receiving exchange data
     */
    struct PaketHeader
    {
        uint32_t m_magic;
        PaketType m_type;
        NodeId m_sourceNodeId;
        uint32_t m_length;
        // data follows...
    } __attribute__ ((__packed__));

public:
    /**
     * Constructor
     *
     * @param ownNodeId Node id of the currenet instance
     * @param socketPort Port to open the socket on for exchange data
     */
    ExchangeManager(con::NodeId ownNodeId, uint16_t socketPort);

    /**
     * Destructor
     */
    ~ExchangeManager() override;

    /**
     * Generate a new paket id for your data to be exchanged
     */
    PaketType GeneratePaketTypeId();

    /**
     * Add a dispatcher that handles incoming data for the specified
     * paket type
     *
     * @param type Type of paket to handle
     * @param dispatcher Dispatcher to add (caller has to manage memory)
     */
    void AddDispatcher(PaketType type, ExchangeDispatcher* dispatcher)
    {
        std::lock_guard<std::mutex> l(m_dispatcherLock);
        m_dispatcher[type].push_back(dispatcher);
    }

    /**
     * Remove a registered dispatcher
     *
     * @param type Type of paket the dispatcher was handling
     * @param dispatcher Dispatcher to remote from handling that type
     */
    void RemoveDispatcher(PaketType type, ExchangeDispatcher* dispatcher)
    {
        std::lock_guard<std::mutex> l(m_dispatcherLock);

        for (auto it = m_dispatcher[type].begin();
            it != m_dispatcher[type].end(); it++) {
            if (*it == dispatcher) {
                m_dispatcher[type].erase(it);
                break;
            }
        }
    }

    /**
     * Send exchange data to a target node. Note that the data is sent
     * over an unreliable connection. Thus, there is no guarantee it will
     * arrive.
     *
     * @param type Type of the paket to send
     * @param targetIPV4 IPV4 address of the target node
     * @param data Data to send (caller has to manage buffer memory).
     *        NULL is valid if you don't have any payload to send
     * @param length Length of the data to send
     */
    void SendData(PaketType type, uint32_t targetIPV4,
        const void* data = nullptr, uint32_t length = 0);

protected:
    void _RunLoop() override;

private:
    const con::NodeId m_ownNodeId;

    sys::SocketUDP* m_socket;
    uint8_t* m_recvBuffer;

    bool m_noDataAvailable;

    uint8_t m_paketTypeIdCounter;
    std::mutex m_dispatcherLock;
    std::vector<std::vector<ExchangeDispatcher*>> m_dispatcher;
};

}
}

#endif //IBNET_CON_EXCHANGEMANAGER_H
