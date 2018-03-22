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

#ifndef IBNET_MSGRC_RECVWORKREQUESTPOOL_H
#define IBNET_MSGRC_RECVWORKREQUESTPOOL_H

#include "ibnet/msgrc/RecvWorkRequest.h"

namespace ibnet {
namespace msgrc {

/**
 * Pool for RecvWorkRequests, single threaded i.e. not thread save (only used by RecvDispatcher)
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 22.03.2018
 */
class RecvWorkRequestPool
{
public:
    /**
     * Constructor
     *
     * @param numWorkRequests Total number of WRQs for the pool
     * @param numSges Num of SGEs of the SGE list for each WRQ
     */
    RecvWorkRequestPool(uint32_t numWorkRequests, uint32_t numSges);

    /**
     * Destructor
     */
    ~RecvWorkRequestPool();

    /**
     * Get a WRQ from the pool
     *
     * @return Pointer to a WRQ (or null if pool empty). Caller does not have to manage memory but must return
     *         the WRQ later using Push.
     */
    RecvWorkRequest* Pop();

    /**
     * Return a WRQ back to the pool
     *
     * @param refWorkRequest WRQ to return to the pool
     */
    void Push(RecvWorkRequest* refWorkRequest);

private:
    const uint32_t m_poolSize;
    const uint32_t m_poolSizeGapped;

    uint32_t m_front;
    uint32_t m_back;

    RecvWorkRequest** m_pool;
    RecvWorkRequest** m_queue;
};

}
}

#endif //IBNET_MSGRC_RECVWORKREQUESTPOOL_H
