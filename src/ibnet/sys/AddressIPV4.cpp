#include "AddressIPV4.h"

#include <arpa/inet.h>

#include "ibnet/sys/StringUtils.h"
#include "ibnet/sys/SystemException.h"

namespace ibnet {
namespace sys {

AddressIPV4::AddressIPV4(void) :
    m_address(INVALID_ADDRESS),
    m_port(INVALID_PORT)
{
    __ToString(m_address);
}

AddressIPV4::AddressIPV4(uint32_t address) :
    m_address(address),
    m_port(INVALID_PORT)
{
    __ToString(m_address);
}

AddressIPV4::AddressIPV4(uint32_t address, uint16_t port) :
    m_address(address),
    m_port(port)
{
    __ToString(m_address);
}

AddressIPV4::AddressIPV4(const std::string& address)
{
    __ToAddressAndPort(address);
}

AddressIPV4::~AddressIPV4(void)
{

}

void AddressIPV4::__ToString(uint32_t address)
{
    m_addressStr += std::to_string((address >> 24) & 0xFF) + ".";
    m_addressStr += std::to_string((address >> 16) & 0xFF) + ".";
    m_addressStr += std::to_string((address >> 8) & 0xFF) + ".";
    m_addressStr += std::to_string(address & 0xFF);
}

void AddressIPV4::__ToAddressAndPort(const std::string& address)
{
    std::vector<std::string> tokens = StringUtils::Split(address, ":");

    if (tokens.size() > 2) {
        throw SystemException("Invalid address format: " + address);
    }

    if (tokens.size() == 0) {
        m_address = INVALID_ADDRESS;
        m_port = INVALID_PORT;
        return;
    }

    struct in_addr addr;

    if (!inet_pton(AF_INET, tokens[0].c_str(), &addr)) {
        throw SystemException("Invalid address format: " + address);
    }

    m_address = ntohl(addr.s_addr);
    m_addressStr = tokens[0];

    if (tokens.size() == 2) {
        m_port = (uint16_t) std::stoi(tokens[1]);
    }
}

}
}