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

#ifndef IBNET_CON_NODECONF_H
#define IBNET_CON_NODECONF_H

#include <iostream>
#include <vector>

#include "ibnet/sys/AddressIPV4.h"

namespace ibnet {
namespace con {

/**
 * Node configuration for IbNodeManager
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class NodeConf
{
public:
    /**
     * Single entry of the node configuration
     */
    class Entry
    {
    public:
        /**
         * Constructor
         */
        Entry();

        /**
         * Constructor
         *
         * @param ipv4 IPV4 of the host
         */
        explicit Entry(const sys::AddressIPV4& ipv4);

        /**
         * Constructor
         *
         * @param hostname Name of the host
         */
        explicit Entry(const std::string& hostname);

        /**
         * Destructor
         */
        ~Entry() = default;

        /**
         * Get the hostname
         */
        const std::string& GetHostname() const
        {
            return m_hostname;
        }

        /**
         * Get the IPV4 address of the host
         */
        const sys::AddressIPV4& GetAddress() const
        {
            return m_address;
        }

        /**
         * Enable output to an out stream
         */
        friend std::ostream& operator<<(std::ostream& os, const Entry& o)
        {
            return os << o.m_hostname << ": " << o.m_address;
        }

    private:
        std::string m_hostname;
        sys::AddressIPV4 m_address;
    };

    /**
     * Constructor
     */
    NodeConf() = default;

    /**
     * Desturctor
     */
    ~NodeConf() = default;

    /**
     * Add a new host
     *
     * @param hostname Host to add
     */
    void AddEntry(const std::string& hostname);

    /**
     * Get all entries
     */
    const std::vector<Entry>& GetEntries() const
    {
        return m_entries;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream& operator<<(std::ostream& os, const NodeConf& o)
    {
        os << "IbNodeConf (" << o.m_entries.size() << "): ";

        for (auto& it : o.m_entries) {
            os << it << ", ";
        }

        return os;
    }

private:
    std::vector<Entry> m_entries;
};

}
}

#endif // IBNET_CON_NODECONF_H
