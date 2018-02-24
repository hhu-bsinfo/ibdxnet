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

#include "JobQueue.h"

#include <thread>

namespace ibnet {
namespace con {

JobQueue::JobQueue(uint32_t size) :
    m_size(size),
    m_front(0),
    m_back(0),
    m_backRes(0),
    m_queue(new Job* [size])
{

}

JobQueue::~JobQueue()
{
    uint32_t front = m_front.load(std::memory_order_relaxed);
    uint32_t back = m_back.load(std::memory_order_relaxed);

    while (front % m_size != back % m_size) {
        delete m_queue[front];
        front++;
    }

    delete[] m_queue;
}

bool JobQueue::PushBack(Job* job)
{
    uint32_t backRes = m_backRes.load(std::memory_order_relaxed);
    uint32_t front;

    while (true) {
        front = m_front.load(std::memory_order_relaxed);

        if (backRes + 1 % m_size == front % m_size) {
            return false;
        }

        if (m_backRes.compare_exchange_weak(backRes, backRes + 1,
            std::memory_order_relaxed)) {
            m_queue[backRes % m_size] = job;

            // wait for any preceding reservations to complete before updating
            // back
            while (!m_back.compare_exchange_weak(backRes, backRes + 1,
                std::memory_order_release)) {
                std::this_thread::yield();
            }

            return true;
        }
    }
}

JobQueue::Job* JobQueue::PopFront()
{
    Job* job;
    uint32_t front = m_front.load(std::memory_order_relaxed);
    uint32_t back = m_back.load(std::memory_order_relaxed);

    if (front % m_size == back % m_size) {
        return nullptr;
    }

    job = m_queue[front % m_size];

    m_front.fetch_add(1, std::memory_order_release);
    return job;
}

bool JobQueue::IsEmpty() const
{
    uint32_t front = m_front.load(std::memory_order_relaxed);
    uint32_t back = m_back.load(std::memory_order_relaxed);

    return front % m_size == back % m_size;
}

}
}