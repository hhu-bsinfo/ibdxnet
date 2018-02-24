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

#ifndef IBNET_CON_JOBTHREAD_H
#define IBNET_CON_JOBTHREAD_H

#include <condition_variable>
#include "ibnet/sys/ThreadLoop.h"

#include "JobDispatcher.h"
#include "JobQueue.h"

namespace ibnet {
namespace con {

/**
 * The JobManager is running a dedicated worker thread which dispatches
 * queued jobs to a set of pre-registered dispatchers (callbacks).
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
class JobManager : public sys::ThreadLoop
{
public:
    /**
     * Constructor
     */
    JobManager();

    /**
     * Destructor
     */
    ~JobManager() override;

    /**
     * Generate a new tyoe id for a job to add to the queue
     */
    JobQueue::JobType GenerateJobTypeId();

    /**
     * Add a dispatcher which gets called if a the target job type is
     * getting executed.
     *
     * @param type Type of the job to register for
     * @param dispatcher Pointer to a dispatcher which gets called if the
     *        target job type is executed (caller has to manage memory)
     */
    void AddDispatcher(JobQueue::JobType type, JobDispatcher* dispatcher)
    {
        std::lock_guard<std::mutex> l(m_dispatcherLock);
        m_dispatcher[type].push_back(dispatcher);
    }

    /**
     * Remove an already registered dispatcher
     *
     * @param type Type of the job the dispatcher is registered for
     * @param dispatcher Dispatcher to remove from the list (caller has to manage memory)
     */
    void RemoveDispatcher(JobQueue::JobType type, JobDispatcher* dispatcher)
    {
        std::lock_guard<std::mutex> l(m_dispatcherLock);

        for (auto it = m_dispatcher[type].begin();
            it != m_dispatcher[type].end(); it++) {
            if (*it == dispatcher) {
                m_dispatcher[type].erase(it);
                break;
            }
        }
    }

    /**
     * Add a job to the queue
     *
     * @param job Job to add. The dedicated work thread is executing
     *        the job once it reaches the front of the queue. Memory
     *        allocated for the job is managed and free'd by the
     *        JobManager.
     */
    void AddJob(JobQueue::Job* job);

    /**
     * Set an idle job which is executed periodically when the job
     * queue is empty.
     *
     * @param job Idle job to set. NULL is valid and removes a
     *        previously set idle job. Memory allocated for the
     *        job is managed and free'd by the JobManager.
     */
    void SetIdleJob(JobQueue::Job* job)
    {
        std::lock_guard<std::mutex> l(m_idleJobLock);
        delete m_idleJob;
        m_idleJob = job;
    }

    /**
     * Check if the job queue is empty
     */
    bool IsQueueEmpty();

protected:
    void _RunLoop() override;

private:
    JobQueue m_queue;
    JobQueue::Job* m_idleJob;

    uint8_t m_jobIdTypeCounter;
    std::mutex m_dispatcherLock;
    std::vector<std::vector<JobDispatcher*>> m_dispatcher;

    std::mutex m_idleJobLock;
    std::mutex m_jobLock;
    std::condition_variable m_jobCondition;
};

}
}

#endif //IBNET_CON_JOBTHREAD_H
