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

#include "RecvWorkRequestPool.h"

#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace msgrc {

RecvWorkRequestPool::RecvWorkRequestPool(uint32_t numWorkRequests, uint32_t numSges) :
    m_poolSize(numWorkRequests),
    m_poolSizeGapped(m_poolSize + 1),
    m_front(0),
    m_back(m_poolSize),
    m_pool(new RecvWorkRequest*[m_poolSize]),
    m_queue(new RecvWorkRequest*[m_poolSizeGapped])
{
    // fill queue with pooled work requests
    for (uint32_t i = 0; i < m_poolSize; i++) {
        m_pool[i] = new RecvWorkRequest(numSges);
        m_queue[i] = m_pool[i];
    }

    // gap
    m_queue[m_poolSize] = nullptr;

    // last entry of queue is used as a gap to be able to determine if a queue is empty or full
}

RecvWorkRequestPool::~RecvWorkRequestPool()
{
    for (uint32_t i = 0; i < m_poolSize; i++) {
        delete m_pool[i];
    }

    delete [] m_pool;
    delete [] m_queue;
}

RecvWorkRequest* RecvWorkRequestPool::Pop()
{
    if (m_front == m_back) {
        IBNET_LOG_WARN("Pool ran dry, m_front %d, m_back %d", m_front, m_back);
        // this should never happen, otherwise pool size incorrect
        throw new sys::IllegalStateException("Pool ran dry, m_front %d, m_back %d", m_front, m_back);
    }

    RecvWorkRequest* tmp = m_queue[m_front];

    if (tmp == nullptr) {
        IBNET_LOG_WARN("Pop null element, m_front %d, m_back %d", m_front, m_back);
        throw new sys::IllegalStateException("Pop null element, m_front %d, m_back %d", m_front, m_back);
    }

    m_queue[m_front] = nullptr;
    m_front = (m_front + 1) % m_poolSizeGapped;

    return tmp;
}

void RecvWorkRequestPool::Push(RecvWorkRequest* refWorkRequest)
{
    if ((m_back + 1) % m_poolSizeGapped == m_front) {
        // should not happen, otherwise, management failure
        IBNET_LOG_WARN("Pool overflow, m_front %d, m_back %d", m_front, m_back);
        throw new sys::IllegalStateException("Pool overflow, m_front %d, m_back %d", m_front, m_back);
    }

    if (m_queue[m_back] != nullptr) {
        IBNET_LOG_WARN("Pushing to index with non null element, m_front %d, m_back %d",
            m_front, m_back);
        throw new sys::IllegalStateException("Pushing to index with non null element, m_front %d, m_back %d",
            m_front, m_back);
    }

    m_queue[m_back] = refWorkRequest;
    m_back = (m_back + 1) % m_poolSizeGapped;
}

}
}