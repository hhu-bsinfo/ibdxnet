#ifndef IBNET_SYS_ADDRESSIPV4_H
#define IBNET_SYS_ADDRESSIPV4_H

#include <cstdint>
#include <iostream>
#include <string>

namespace ibnet {
namespace sys {

class AddressIPV4
{
public:
    static const uint32_t INVALID_ADDRESS = (const uint32_t) -1;
    static const uint16_t INVALID_PORT = (const uint16_t) -1;

    AddressIPV4(void);

    AddressIPV4(uint32_t address);

    AddressIPV4(uint32_t address, uint16_t port);

    AddressIPV4(const std::string& address);

    ~AddressIPV4(void);

    uint32_t GetAddress(void) const {
        return m_address;
    }

    uint16_t GetPort(void) const {
        return m_port;
    }

    bool IsValid(void) {
        return m_address != INVALID_ADDRESS;
    }

    const std::string GetAddressStr(bool withPort = false) const {
        if (withPort) {
            return m_addressStr + ":" + std::to_string(m_port);
        } else {
            return m_addressStr;
        }
    }

    friend std::ostream &operator<<(std::ostream& os, const AddressIPV4& o) {
        return os << o.m_addressStr;
    }

private:
    uint32_t m_address;
    uint16_t m_port;
    std::string m_addressStr;

    void __ToString(uint32_t address);
    void __ToAddressAndPort(const std::string& address);
};

}
}

#endif //IBNET_SYS_ADDRESSIPV4_H
