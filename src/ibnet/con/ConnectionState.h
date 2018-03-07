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

#ifndef IBNET_CON_CONNECTIONSTATE_H
#define IBNET_CON_CONNECTIONSTATE_H

#include <atomic>
#include <bitset>
#include <cstdint>
#include <ostream>

namespace ibnet {
namespace con {

/**
 * State and state management of a connection. This includes various
 * stages of state a connection has to go through when creating/establishing
 * a new connection or closing an existing one.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.02.2018
 */
struct ConnectionState
{
    static const int32_t CONNECTION_NOT_AVAILABLE = INT32_MIN;
    static const int32_t CONNECTION_AVAILABLE = 0;
    static const int32_t CONNECTION_CLOSING = INT32_MIN / 2;

    /**
     * States of a single connection
     */
    enum State
    {
        e_StateNotAvailable = 0,
        e_StateInCreation = 1,
        e_StateCreated = 2,
        e_StateConnected = 3,
    };

    /**
     * Flags for connection data exchange and synchronization
     */
    enum ExchgFlag
    {
        e_ExchgFlagConnectedToRemote = (1 << 0),
        e_ExchgFlagRemoteConnected = (1 << 1),
        e_ExchgFlagCompleted =
        e_ExchgFlagConnectedToRemote | e_ExchgFlagRemoteConnected,
    };

    std::atomic<uint8_t> m_state;
    std::atomic<uint8_t> m_exchgFlags;
    // we have to to store the previously received remote exchg state as well
    // in order to determine when to terminate the connection data exchange
    // and being certain that all data is exchanged
    std::atomic<uint8_t> m_remoteExchgFlags;
    std::atomic<int32_t> m_available;

    /**
     * Constructor
     */
    ConnectionState() :
            m_state(e_StateNotAvailable),
            m_exchgFlags(0),
            m_remoteExchgFlags(0),
            m_available(CONNECTION_NOT_AVAILABLE)
    {
    }

    /**
     * Check if the connection is fully connected
     */
    bool IsConnected() const
    {
        return m_state.load(std::memory_order_relaxed) == e_StateConnected;
    }

    /**
     * Check if a remote is already connected to the current instance
     *
     * @param remoteState Connection state of the remote
     */
    static bool IsRemoteConnectedToCurrent(uint8_t remoteState)
    {
        return remoteState & e_ExchgFlagConnectedToRemote;
    }

    /**
     * Check if the connection exchange is fully complete (current
     * instance and remote have exchanged all data)
     */
    bool ConnectionExchgComplete() const
    {
        return m_exchgFlags.load(std::memory_order_relaxed) ==
                e_ExchgFlagCompleted && m_remoteExchgFlags.load(
                std::memory_order_relaxed) == e_ExchgFlagCompleted;
    }

    /**
     * Check if the current instance is connecte to the remote
     */
    bool IsConnectedToRemote() const
    {
        return __IsFlagSet(e_ExchgFlagConnectedToRemote);
    }

    /**
     * Check if the remote is connected to the current instance
     */
    bool IsRemoteConnected() const
    {
        return __IsFlagSet(e_ExchgFlagRemoteConnected);
    }

    /**
     * Set the state that the current instance is connected to the remote
     */
    void SetConnectedToRemote()
    {
        __FlagConnectionExchgState(e_ExchgFlagConnectedToRemote);
    }

    /**
     * Set the state that the remote is connected to the current instance
     */
    void SetRemoteConnected()
    {
        __FlagConnectionExchgState(e_ExchgFlagRemoteConnected);
    }

    /**
     * Operator<< to print to ostreams
     *
     * @param os Target ostream
     * @param o State object to output
     * @return Target ostream
     */
    friend std::ostream& operator<<(std::ostream& os, const ConnectionState& o)
    {
        return os << static_cast<uint16_t>(
                o.m_state.load(std::memory_order_relaxed))
                << "|" <<
                std::bitset<2>(o.m_exchgFlags.load(std::memory_order_relaxed))
                << "|" << std::bitset<2>(
                o.m_remoteExchgFlags.load(std::memory_order_relaxed))
                << "|" << std::dec << o.m_available.load(std::memory_order_relaxed);
    }

private:
    void __FlagConnectionExchgState(uint8_t flag)
    {
        uint8_t prev = m_exchgFlags.load(std::memory_order_relaxed);
        uint8_t newVal;

        do {
            newVal = prev | flag;
        } while (!m_exchgFlags.compare_exchange_strong(prev, newVal,
                std::memory_order_relaxed));
    }

    bool __IsFlagSet(uint8_t flag) const
    {
        return m_exchgFlags.load(std::memory_order_relaxed) & flag;
    }

    bool __IsFlagRemoteSet(uint8_t flag) const
    {
        return m_remoteExchgFlags.load(std::memory_order_relaxed) & flag;
    }
};

}
}

#endif //IBNET_CON_CONNECTIONSTATE_H
