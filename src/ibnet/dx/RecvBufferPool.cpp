/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
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
    m_bufferPoolSize(totalPoolSize / recvBufferSize),
    m_bufferSize(recvBufferSize),
    m_refProtDom(refProtDom),
    m_dataBuffersFront(0),
    m_dataBuffersBack(
        (uint32_t) (totalPoolSize / recvBufferSize - 1)),
    m_dataBuffersBackRes(
        (uint32_t) (totalPoolSize / recvBufferSize - 1)),
    m_insufficientBufferCounter(0)
{
    // allocate a single region and slice it into multiple buffers for the pool

    m_memoryPool = new core::IbMemReg(
        aligned_alloc(static_cast<size_t>(getpagesize()),
        m_bufferPoolSize * m_bufferSize), m_bufferPoolSize * m_bufferSize,
        true);

    IBNET_LOG_INFO("Allocated memory pool region %p, size %d",
        m_memoryPool->GetAddress(), m_memoryPool->GetSize());

    m_refProtDom->Register(m_memoryPool);

    IBNET_LOG_INFO("Allocating %d data buffers, size %d each for total pool "
        "size %d...", m_bufferPoolSize, recvBufferSize, totalPoolSize);

    m_dataBuffers = new core::IbMemReg*[m_bufferPoolSize];

    for (uint32_t i = 0; i < m_bufferPoolSize; i++) {
        m_dataBuffers[i] = new core::IbMemReg((void*)
            (((uintptr_t) m_memoryPool->GetAddress()) + i * recvBufferSize),
            recvBufferSize, m_memoryPool);
    }

    IBNET_LOG_INFO("Allocation finished");
}

RecvBufferPool::~RecvBufferPool()
{
    m_refProtDom->Deregister(m_memoryPool);
    delete m_memoryPool;

    for (uint32_t i = 0; i < m_bufferPoolSize; i++) {
        delete m_dataBuffers[i];
    }

    delete [] m_dataBuffers;
}

core::IbMemReg* RecvBufferPool::GetBuffer()
{
    core::IbMemReg* buffer = nullptr;

    uint32_t front = m_dataBuffersFront.load(std::memory_order_relaxed);
    uint32_t back;

    while (true) {
        back = m_dataBuffersBack.load(std::memory_order_relaxed);

        if (front % m_bufferPoolSize == back % m_bufferPoolSize) {
            uint64_t counter = m_insufficientBufferCounter.fetch_add(1,
                std::memory_order_relaxed);

            if (counter % 1000000 == 0) {
                IBNET_LOG_WARN("Insufficient pooled incoming buffers... "
                    "waiting for buffers to get returned. If this warning "
                    "appears periodically and very frequently, consider "
                    "increasing the receive pool's total size to avoid "
                    "possible performance penalties, counter: %d", counter);
            }

            std::this_thread::yield();

            continue;
        }

        buffer = m_dataBuffers[front % m_bufferPoolSize];

        m_dataBuffersFront.fetch_add(1, std::memory_order_release);

        break;
    }

    return buffer;
}

uint32_t RecvBufferPool::GetBuffers(core::IbMemReg** retBuffers,
        uint32_t count)
{
    uint32_t front = m_dataBuffersFront.load(std::memory_order_relaxed);
    uint32_t back;

    while (true) {
        back = m_dataBuffersBack.load(std::memory_order_relaxed);

        if (front % m_bufferPoolSize == back % m_bufferPoolSize) {
            uint64_t counter = m_insufficientBufferCounter.fetch_add(1,
                std::memory_order_relaxed);

            if (counter % 1000000 == 0) {
                IBNET_LOG_WARN("Insufficient pooled incoming buffers... "
                    "waiting for buffers to get returned. If this warning "
                    "appears periodically and very frequently, consider "
                    "increasing the receive pool's total size to avoid "
                    "possible performance penalties, counter: %d", counter);
            }

            std::this_thread::yield();

            continue;
        }

        uint32_t available;

        if (front <= back) {
            available = back - front;
        } else {
            available = 0xFFFFFFFF - front + back;
        }

        if (available < count) {
            count = available;
        }

        for (uint32_t i = 0; i < count; i++) {
            retBuffers[i] = m_dataBuffers[(front + i) % m_bufferPoolSize];
        }

        m_dataBuffersFront.fetch_add(count, std::memory_order_release);

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

        if ((backRes + 1) % m_bufferPoolSize == front % m_bufferPoolSize) {
            throw sys::IllegalStateException(
                "Pool overflow, this should not happen: backRes %d, front %d",
                backRes, front);
        }

        if (m_dataBuffersBackRes.compare_exchange_weak(backRes, backRes + 1,
                std::memory_order_relaxed)) {
            m_dataBuffers[backRes % m_bufferPoolSize] = buffer;

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
                    backRes + 1, std::memory_order_release)) {
                std::this_thread::yield();
            }

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

        if ((backRes + count) % m_bufferPoolSize == front % m_bufferPoolSize) {
            throw sys::IllegalStateException(
                "Pool overflow, this should not happen: backRes %d, front %d, "
                "count %d", backRes, front, count);
        }

        if (m_dataBuffersBackRes.compare_exchange_weak(backRes, backRes + count,
                std::memory_order_relaxed)) {

            for (uint32_t i = 0; i < count; i++) {
                m_dataBuffers[(backRes + i) % m_bufferPoolSize] = buffers[i];
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
                    backRes + count, std::memory_order_release)) {
                std::this_thread::yield();
            }

            break;
        }
    }
}

void RecvBufferPool::ReturnBuffers(void* ptrFirstBuffer, uint32_t stride,
        uint32_t count)
{
    throw sys::IllegalStateException(
        "This method is still buggy (creates memory misalignment");

    if (count == 0) {
        return;
    }

    uint32_t backRes = m_dataBuffersBackRes.load(std::memory_order_relaxed);
    uint32_t front;

    while (true) {
        front = m_dataBuffersFront.load(std::memory_order_relaxed);

        if ((backRes + count) % m_bufferPoolSize == front % m_bufferPoolSize) {
            throw sys::IllegalStateException(
                "Pool overflow, this should not happen: backRes %d, front %d, "
                    "count %d", backRes, front, count);
        }

        if (m_dataBuffersBackRes.compare_exchange_weak(backRes, backRes + count,
            std::memory_order_relaxed)) {

            for (uint32_t i = 0; i < count; i++) {
                auto* buffer = (core::IbMemReg*)
                    ((uintptr_t ) ptrFirstBuffer) + i * stride;
                m_dataBuffers[(backRes + i) % m_bufferPoolSize] = buffer;
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
                backRes + count, std::memory_order_release)) {
                std::this_thread::yield();
            }

            break;
        }
    }
}

}
}