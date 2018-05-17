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

#include "RecvBufferPool.h"

#include "ibnet/sys/IllegalStateException.h"
#include "ibnet/sys/Logger.hpp"

namespace ibnet {
namespace dx {

RecvBufferPool::RecvBufferPool(uint64_t totalPoolSize,
        uint32_t recvBufferSize, core::IbProtDom* refProtDom) :
        m_bufferPoolSize(
                static_cast<const uint32_t>(totalPoolSize / recvBufferSize)),
        m_bufferPoolSizeGapped(m_bufferPoolSize + 1),
        m_bufferSize(recvBufferSize),
        m_refProtDom(refProtDom),
        m_dataBuffersFront(0),
        m_dataBuffersBack(m_bufferPoolSize),
        m_dataBuffersBackRes(m_bufferPoolSize),
        m_insufficientBufferCounter(0)
{
    // allocate a single region and slice it into multiple buffers for the pool

    m_memoryPool = new core::IbMemReg(
            aligned_alloc(static_cast<size_t>(getpagesize()),
                static_cast<uint64_t>(m_bufferPoolSize) * m_bufferSize),
                static_cast<uint64_t>(m_bufferPoolSize) * m_bufferSize,
            true);

    IBNET_LOG_INFO("Allocated memory pool region %p, size %d",
            m_memoryPool->GetAddress(), m_memoryPool->GetSize());

    m_refProtDom->Register(m_memoryPool);

    IBNET_LOG_INFO("Allocating %d data buffers, size %d each for total pool "
            "size %d...", m_bufferPoolSize, recvBufferSize, totalPoolSize);

    // This is just used to keep track of the pool entries and not when
    // handing out and returning entries
    m_bufferPool = new core::IbMemReg* [m_bufferPoolSize];

    // size +1: we need a gap entry for the front and back pointers to avoid
    // overlapping if pool is full. otherwise, we can't determine if a pool
    // is full or empty. Here: if front == back: pool empty
    m_dataBuffers = new core::IbMemReg* [m_bufferPoolSizeGapped];

    for (uint32_t i = 0; i < m_bufferPoolSize; i++) {
        m_bufferPool[i] = new core::IbMemReg((void*)
                        (((uintptr_t) m_memoryPool->GetAddress()) + static_cast<uint64_t>(i) * recvBufferSize),
                recvBufferSize, m_memoryPool);
        m_dataBuffers[i] = m_bufferPool[i];
    }

    // gap, see above
    m_dataBuffers[m_bufferPoolSize] = nullptr;

    IBNET_LOG_INFO("Allocation finished");
}

RecvBufferPool::~RecvBufferPool()
{
    m_refProtDom->Deregister(m_memoryPool);
    delete m_memoryPool;

    for (uint32_t i = 0; i < m_bufferPoolSize; i++) {
        delete m_bufferPool[i];
    }

    delete[] m_bufferPool;
    delete[] m_dataBuffers;
}

core::IbMemReg* RecvBufferPool::GetBuffer()
{
    core::IbMemReg* buffer = nullptr;

    uint32_t front = m_dataBuffersFront.load(std::memory_order_relaxed);
    uint32_t back;

    while (true) {
        back = m_dataBuffersBack.load(std::memory_order_relaxed);

        if (front == back) {
            uint64_t counter = m_insufficientBufferCounter.fetch_add(1, std::memory_order_relaxed);

            if (counter % 1000000 == 0) {
                IBNET_LOG_WARN("Insufficient pooled incoming buffers... "
                        "waiting for buffers to get returned. If this warning "
                        "appears periodically and very frequently, consider "
                        "increasing the receive pool's total size to avoid "
                        "possible performance penalties, counter: %d", counter);
            }

            return nullptr;
        }

        buffer = m_dataBuffers[front];

        m_nonReturnedBuffers.fetch_add(1, std::memory_order_relaxed);

        if (buffer == nullptr) {
            throw sys::IllegalStateException(
                    "Got invalid (null) buffer from pool, pos %d", front);
        }

        m_dataBuffers[front] = nullptr;

        m_dataBuffersFront.store((front + 1) % m_bufferPoolSizeGapped, std::memory_order_release);

        break;
    }

    return buffer;
}

uint32_t RecvBufferPool::GetBuffers(core::IbMemReg** retBuffers, uint32_t count)
{
    if (count == 0) {
        return 0;
    }

    uint32_t front = m_dataBuffersFront.load(std::memory_order_relaxed);
    uint32_t back;

    while (true) {
        back = m_dataBuffersBack.load(std::memory_order_relaxed);

        if (front == back) {
            uint64_t counter = m_insufficientBufferCounter.fetch_add(1, std::memory_order_relaxed);

            if (counter % 1000000 == 0) {
                IBNET_LOG_WARN("Insufficient pooled incoming buffers... "
                        "waiting for buffers to get returned. If this warning "
                        "appears periodically and very frequently, consider "
                        "increasing the receive pool's total size to avoid "
                        "possible performance penalties, front %d, back %d, counter: %d",
                        front, back, counter);
            }

            return 0;
        }

        uint32_t available;

        if (front <= back) {
            available = back - front;
        } else {
            // not pool size gap size which includes the gap/nullptr
            available = m_bufferPoolSize - front + back;
        }

        if (available < count) {
            count = available;
        }

        for (uint32_t i = 0; i < count; i++) {
            retBuffers[i] = m_dataBuffers[(front + i) % m_bufferPoolSizeGapped];

            if (retBuffers[i] == nullptr) {
                throw sys::IllegalStateException(
                        "Got invalid (null) buffer from pool, pos %d",
                        front % m_bufferSize);
            }

            m_dataBuffers[(front + i) % m_bufferPoolSizeGapped] = nullptr;
        }

        m_nonReturnedBuffers.fetch_add(count, std::memory_order_relaxed);

        m_dataBuffersFront.store((front + count) % m_bufferPoolSizeGapped, std::memory_order_release);

        break;
    }

    return count;
}

void RecvBufferPool::ReturnBuffer(core::IbMemReg* buffer)
{
    uint32_t backRes = m_dataBuffersBackRes.load(std::memory_order_relaxed);
    uint32_t front;

    while (true) {
        front = m_dataBuffersFront.load(std::memory_order_relaxed);

        if ((backRes + 1) % m_bufferPoolSizeGapped == front) {
            throw sys::IllegalStateException(
                    "Pool overflow, this should not happen: backRes %d, front %d",
                    backRes, front);
        }

        if (m_dataBuffersBackRes.compare_exchange_weak(backRes, (backRes + 1) % m_bufferPoolSizeGapped,
                std::memory_order_acquire)) {
            if (m_dataBuffers[backRes]) {
                throw sys::IllegalStateException(
                        "Overwriting existing buffer %p at pos %d with %p",
                        (void*) m_dataBuffers[backRes], backRes, (void*) buffer);
            }

            m_dataBuffers[backRes] = buffer;

            // if two buffers are returned at the same time, the first return
            // could be interrupt by a second return. the reserve of the first
            // return is already completed but the back pointer is not updated.
            // the second return reserves and updates the back pointer. now,
            // the back pointer is pointing to the first returns reserve which
            // might not be completed, yet.
            // solution: the second return has to wait for the first return
            // to complete, both, the reservation and updating of the back
            // pointer before it can update the back pointer as well
            while (!m_dataBuffersBack.compare_exchange_weak(backRes,
                    (backRes + 1) % m_bufferPoolSizeGapped, std::memory_order_release)) {
                std::this_thread::yield();
            }

            m_nonReturnedBuffers.fetch_sub(1, std::memory_order_relaxed);

            break;
        }
    }
}

void RecvBufferPool::ReturnBuffers(core::IbMemReg** buffers, uint32_t count)
{
    if (count == 0) {
        return;
    }

    uint32_t backRes = m_dataBuffersBackRes.load(std::memory_order_relaxed);
    uint32_t front;

    while (true) {
        front = m_dataBuffersFront.load(std::memory_order_relaxed);

        if ((backRes + count) % m_bufferPoolSizeGapped == front) {
            throw sys::IllegalStateException(
                    "Pool overflow, this should not happen: backRes %d, front %d, "
                            "count %d", backRes, front, count);
        }

        if (m_dataBuffersBackRes.compare_exchange_weak(backRes, (backRes + count) % m_bufferPoolSizeGapped,
                std::memory_order_relaxed)) {

            for (uint32_t i = 0; i < count; i++) {
                if (m_dataBuffers[(backRes + i) % m_bufferPoolSizeGapped]) {
                    throw sys::IllegalStateException(
                            "Overwriting existing buffer %p at pos %d with %p",
                            (void*) m_dataBuffers[(backRes + i) %
                                    m_bufferPoolSizeGapped],
                            (backRes + i) % m_bufferPoolSizeGapped,
                            (void*) buffers[i]);
                }

                m_dataBuffers[(backRes + i) % m_bufferPoolSizeGapped] = buffers[i];
            }

            // if two buffers are returned at the same time, the first return
            // could be interrupt by a second return. the reserve of the first
            // return is already completed but the back pointer is not updated.
            // the second return reserves and updates the back pointer. now,
            // the back pointer is pointing to the first returns reserve which
            // might not be completed, yet.
            // solution: the second return has to wait for the first return
            // to complete, both, the reservation and updating of the back
            // pointer before it can update the back pointer as well
            while (!m_dataBuffersBack.compare_exchange_weak(backRes,
                    (backRes + count) % m_bufferPoolSizeGapped, std::memory_order_release)) {
                std::this_thread::yield();
            }

            m_nonReturnedBuffers.fetch_sub(count, std::memory_order_relaxed);

            break;
        }
    }
}

}
}