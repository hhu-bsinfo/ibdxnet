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

#include "SendWorkRequestCtxPool.h"

#include "ibnet/sys/IllegalStateException.h"
#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace msgrc {

SendWorkRequestCtxPool::SendWorkRequestCtxPool(uint32_t numWorkRequests) :
        m_poolSize(numWorkRequests),
        m_nonReturnedBuffers(0),
        m_front(0),
        m_back(0),
        m_pool(new SendWorkRequestCtx* [m_poolSize]),
        m_queue(new SendWorkRequestCtx* [m_poolSize])
{
    // fill queue with pooled work requests
    for (uint32_t i = 0; i < m_poolSize; i++) {
        m_pool[i] = new SendWorkRequestCtx();
        m_queue[i] = m_pool[i];
    }
}

SendWorkRequestCtxPool::~SendWorkRequestCtxPool()
{
    for (uint32_t i = 0; i < m_poolSize; i++) {
        delete m_pool[i];
    }

    delete[] m_pool;
    delete[] m_queue;
}

SendWorkRequestCtx* SendWorkRequestCtxPool::Pop()
{
    if (m_nonReturnedBuffers == m_poolSize) {
        IBNET_LOG_WARN("Pool ran dry, m_front %d, m_back %d", m_front, m_back);
        // this should never happen, otherwise pool size incorrect
        throw new sys::IllegalStateException("Pool ran dry, m_front %d, m_back %d", m_front, m_back);
    }

    SendWorkRequestCtx* tmp = m_queue[m_front];

    if (tmp == nullptr) {
        IBNET_LOG_WARN("Pop null element, m_front %d, m_back %d", m_front, m_back);
        throw new sys::IllegalStateException("Pop null element, m_front %d, m_back %d", m_front, m_back);
    }

    m_queue[m_front] = nullptr;
    m_front = (m_front + 1) % m_poolSize;

    m_nonReturnedBuffers++;

    return tmp;
}

void SendWorkRequestCtxPool::Push(SendWorkRequestCtx* refWorkRequest)
{
    if (m_nonReturnedBuffers == 0) {
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
    m_back = (m_back + 1) % m_poolSize;

    m_nonReturnedBuffers--;
}

}
}