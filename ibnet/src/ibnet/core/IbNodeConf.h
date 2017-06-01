#ifndef IBNET_CORE_IBNODECONF_H
#define IBNET_CORE_IBNODECONF_H

#include <iostream>
#include <vector>

#include "ibnet/sys/AddressIPV4.h"

namespace ibnet {
namespace core {

class IbNodeConf
{
public:
    class Entry
    {
    public:
        Entry(void);
        Entry(const sys::AddressIPV4& ipv4);
        Entry(const std::string& hostname);
        ~Entry(void);

        const std::string& GetHostname(void) const {
            return m_hostname;
        }

        const sys::AddressIPV4& GetAddress(void) const {
            return m_address;
        }

        friend std::ostream &operator<<(std::ostream& os, const Entry& o) {
            return os << o.m_hostname << ": " << o.m_address;
        }

    private:
        std::string m_hostname;
        sys::AddressIPV4 m_address;
    };

    IbNodeConf(void);
    ~IbNodeConf(void);

    void AddEntry(const std::string& hostname);

    const std::vector<Entry>& GetEntries(void) const {
        return m_entries;
    }

    friend std::ostream &operator<<(std::ostream& os, const IbNodeConf& o) {
        os << "IbNodeConf (" << o.m_entries.size() << "):";

        for (auto& it : o.m_entries) {
            os << it;
        }

        return os;
    }

private:
    std::vector<Entry> m_entries;
};

}
}

#endif // IBNET_CORE_IBNODECONF_H
