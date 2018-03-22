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

#ifndef IBNET_MSGRC_RECVWORKREQUEST_H
#define IBNET_MSGRC_RECVWORKREQUEST_H

#include <cstdint>

#include "ibnet/msgrc/ScatterGatherList.h"

namespace ibnet {
namespace msgrc {

/**
 * Wrapper with helper methods for managing receive WRQs
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 22.03.2018
 */
struct RecvWorkRequest
{
    ibv_recv_wr m_recvWr;
    ScatterGatherList m_sgls;

    /**
     * Constructor
     *
     * @param maxSge Max number of SGEs for the SGE list for this WRQ
     */
    RecvWorkRequest(uint32_t maxSge) :
        m_recvWr(),
        m_sgls(maxSge)
    {
    };

    /**
     * Destructor
     */
    ~RecvWorkRequest()
    {
    }

    /**
     * Prepare a work request. This must be called before posting it and after all
     * setting all data for it is finished.
     */
    void Prepare()
    {
        m_recvWr.wr_id = (uint64_t) this;
        m_recvWr.sg_list = m_sgls.m_sgeList;
        m_recvWr.num_sge = m_sgls.m_numUsedElems;
        m_recvWr.next = nullptr;
    }

    /**
     * Chain this WRQ to a successor
     *
     * @param refOther Successor to point to for the WRQ list
     */
    void Chain(RecvWorkRequest* refOther)
    {
        m_recvWr.next = &refOther->m_recvWr;
    }

    /**
     * Enable output to an out stream
     */
    friend std::ostream& operator<<(std::ostream& os, const RecvWorkRequest& o)
    {
        os << "m_recvWr " << std::hex << static_cast<const void*>(&o.m_recvWr) << std::dec;
        os << ", m_sgls:  " << o.m_sgls;

        return os;
    }
};

}
}

#endif //IBNET_MSGRC_RECVWORKREQUEST_H
