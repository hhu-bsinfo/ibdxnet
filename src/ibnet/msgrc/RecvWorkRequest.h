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

struct RecvWorkRequest
{
    ibv_recv_wr m_recvWr;
    ScatterGatherList m_sgls;

    RecvWorkRequest(uint32_t maxSge) :
        m_recvWr(),
        m_sgls(maxSge)
    {
    };

    ~RecvWorkRequest()
    {
    }

    void Prepare()
    {
        m_recvWr.wr_id = (uint64_t) this;
        m_recvWr.sg_list = m_sgls.m_sgeList;
        m_recvWr.num_sge = m_sgls.m_numUsedElems;
        m_recvWr.next = nullptr;
    }

    void Chain(RecvWorkRequest* refOther)
    {
        m_recvWr.next = &refOther->m_recvWr;
    }

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
