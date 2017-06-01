#include "IbNodeConf.h"

#include "ibnet/sys/Network.h"

namespace ibnet {
namespace core {

IbNodeConf::Entry::Entry(void) :
    m_hostname("***INVALID***"),
    m_address()
{

}

IbNodeConf::Entry::Entry(const sys::AddressIPV4& ipv4) :
    m_hostname(),
    m_address(ipv4)
{

}

IbNodeConf::Entry::Entry(const std::string& hostname) :
    m_hostname(hostname),
    m_address(sys::Network::ResolveHostname(m_hostname))
{

}

IbNodeConf::Entry::~Entry(void)
{

}

IbNodeConf::IbNodeConf(void)
{

}

IbNodeConf::~IbNodeConf(void)
{

}

void IbNodeConf::AddEntry(const std::string& hostname)
{
    m_entries.push_back(Entry(hostname));
}

}
}