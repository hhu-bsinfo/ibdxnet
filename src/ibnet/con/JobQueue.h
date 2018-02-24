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

#ifndef IBNET_CON_JOBQUEUE_H
#define IBNET_CON_JOBQUEUE_H

#include <atomic>
#include <cstdint>
#include <cstring>

namespace ibnet {
namespace con {

/**
 * FIFO queue for the JobManager
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
class JobQueue
{
public:
    typedef uint8_t JobType;
    static const JobType JOB_TYPE_INVALID = 0xFF;

    /**
     * Base for a job to execute
     */
    struct Job
    {
        const JobType m_type;

        explicit Job(JobType type) :
            m_type(type)
        {
        }

        virtual ~Job() = default;
    };

public:
    /**
     * Constructor
     *
     * @param size Total size of the queue
     */
    explicit JobQueue(uint32_t size);

    /**
     * Destructor
     */
    ~JobQueue();

    /**
     * Add a job to the queue
     *
     * @param job Job to add
     * @return True if adding successful, false if queue full
     */
    bool PushBack(Job* job);

    /**
     * Get a job from the front of the queue
     *
     * @return Pointer to a job or NULL if queue empty
     */
    Job* PopFront();

    /**
     * Check if the queue is empty
     */
    bool IsEmpty() const;

private:
    const uint32_t m_size;

    std::atomic<uint32_t> m_front;
    std::atomic<uint32_t> m_back;
    std::atomic<uint32_t> m_backRes;

    Job** m_queue;
};

}
}

#endif //IBNET_CON_JOBQUEUE_H
