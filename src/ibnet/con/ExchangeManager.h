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

//
// Created by nothaas on 1/29/18.
//
class ExchangeManager : public sys::ThreadLoop
{
public:
    typedef uint8_t PaketType;
    static const uint8_t INVALID_PACKET_TYPE = 0xFF;

    static const size_t MAX_PAKET_SIZE = 1024 * 64;

public:
    struct PaketHeader {
        uint32_t m_magic;
        PaketType m_type;
        NodeId m_sourceNodeId;
        uint32_t m_length;
        // data follows...
    } __attribute__ ((__packed__));

public:
    ExchangeManager(con::NodeId ownNodeId, uint16_t socketPort);

    ~ExchangeManager() override;

    PaketType GeneratePaketTypeId();

    void AddDispatcher(PaketType type, ExchangeDispatcher* dispatcher) {
        std::lock_guard<std::mutex> l(m_dispatcherLock);
        m_dispatcher[type].push_back(dispatcher);
    }

    void RemoveDispatcher(PaketType type, ExchangeDispatcher* dispatcher) {
        std::lock_guard<std::mutex> l(m_dispatcherLock);

        for (auto it = m_dispatcher[type].begin();
                it != m_dispatcher[type].end(); it++) {
            if (*it == dispatcher) {
                m_dispatcher[type].erase(it);
                break;
            }
        }
    }

    // data null is valid if you don't have any payload
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
