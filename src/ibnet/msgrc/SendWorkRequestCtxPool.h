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

#ifndef IBNET_SENDWORKREQUESTCTXPOOL_H
#define IBNET_SENDWORKREQUESTCTXPOOL_H

#include "SendWorkRequestCtx.h"

namespace ibnet {
namespace msgrc {

class SendWorkRequestCtxPool
{
public:
    /**
     * Constructor
     *
     * @param numWorkRequests Total number of WRQs for the pool
     */
    SendWorkRequestCtxPool(uint32_t numWorkRequests);

    /**
     * Destructor
     */
    ~SendWorkRequestCtxPool();

    /**
     * Get a WRQ ctx from the pool
     *
     * @return Pointer to a WRQ ctx (or null if pool empty). Caller does not have to manage memory but must return
     *         the WRQ ctx later using Push.
     */
    SendWorkRequestCtx* Pop();

    /**
     * Return a WRQ ctx back to the pool
     *
     * @param refWorkRequest WRQ ctx to return to the pool
     */
    void Push(SendWorkRequestCtx* refWorkRequest);

    /**
     * Overloading << operator for printing to ostreams
     *
     * @param os Ostream to output to
     * @param o Operation to generate output for
     * @return Ostream object
     */
    friend std::ostream& operator<<(std::ostream& os, const SendWorkRequestCtxPool& o)
    {
        int64_t nonReturnedBuffers = o.m_nonReturnedBuffers;
        uint32_t front = o.m_front;
        uint32_t back = o.m_back;

        uint32_t avail = 0;

        if (front <= back) {
            avail = back - front;
        } else {
            avail = o.m_poolSize - front + back;
        }

        os << "nonReturnedBuffers " << nonReturnedBuffers << ", front " << front << ", back " << back <<
                ", avail " << avail;

        return os;
    }

private:
    const uint32_t m_poolSize;

    uint64_t m_nonReturnedBuffers;

    uint32_t m_front;
    uint32_t m_back;

    SendWorkRequestCtx** m_pool;
    SendWorkRequestCtx** m_queue;
};

}
}

#endif //IBNET_SENDWORKREQUESTCTXPOOL_H
