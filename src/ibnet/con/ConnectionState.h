//
// Created by nothaas on 2/7/18.
//

#ifndef IBNET_CON_CONNECTIONSTATE_H
#define IBNET_CON_CONNECTIONSTATE_H

#include <atomic>
#include <cstdint>

namespace ibnet {
namespace con {

struct ConnectionState
{
    static const int32_t CONNECTION_NOT_AVAILABLE = INT32_MIN;
    static const int32_t CONNECTION_AVAILABLE = 0;
    static const int32_t CONNECTION_CLOSING = INT32_MIN / 2;

    enum State {
        e_StateNotAvailable = 0,
        e_StateInCreation = 1,
        e_StateCreated = 2,
        e_StateConnected = 3,
    };

    enum ExchgFlag {
        e_ExchgFlagConnectedToRemote = (1 << 0),
        e_ExchgFlagRemoteConnected = (1 << 1),
    };

    std::atomic<uint8_t> m_state;
    std::atomic<uint8_t> m_exchgFlags;
    std::atomic<int32_t> m_available;

    ConnectionState() :
        m_state(e_StateNotAvailable),
        m_exchgFlags(0),
        m_available(CONNECTION_NOT_AVAILABLE)
    {}

    bool IsConnected() const {
        return m_state.load(std::memory_order_relaxed) == e_StateConnected;
    }

    static bool IsRemoteConnected(uint8_t remoteState) {
        return remoteState & e_ExchgFlagConnectedToRemote;
    }

    bool ConnectionExchgComplete() const {
        return __IsFlagSet(e_ExchgFlagConnectedToRemote |
            e_ExchgFlagRemoteConnected);
    }

    bool IsConnectedToRemote() const {
        return __IsFlagSet(e_ExchgFlagConnectedToRemote);
    }

    bool IsRemoteConnected() const {
        return __IsFlagSet(e_ExchgFlagRemoteConnected);
    }

    void SetConnectedToRemote() {
        __FlagConnectionExchgState(e_ExchgFlagConnectedToRemote);
    }

    void SetRemoteConnected() {
        __FlagConnectionExchgState(e_ExchgFlagRemoteConnected);
    }

    friend std::ostream &operator<<(std::ostream& os,
        const ConnectionState& o) {
        return os << o.m_state.load(std::memory_order_relaxed) << "|" <<
            std::hex << o.m_exchgFlags.load(std::memory_order_relaxed) << "|" <<
            std::dec << o.m_available.load(std::memory_order_relaxed);
    }

private:
    void __FlagConnectionExchgState(uint8_t flag) {
        uint8_t prev = m_exchgFlags.load(std::memory_order_relaxed);
        uint8_t newVal;

        do {
            newVal = prev | flag;
        } while (m_exchgFlags.compare_exchange_strong(prev, newVal,
            std::memory_order_relaxed));
    }

    bool __IsFlagSet(uint8_t flag) const {
        return m_exchgFlags.load(std::memory_order_relaxed) & flag;
    }
};

}
}

#endif //IBNET_CON_CONNECTIONSTATE_H