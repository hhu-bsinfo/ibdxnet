//
// Created by nothaas on 1/29/18.
//

#include "JobManager.h"

#include "ibnet/sys/IllegalStateException.h"

namespace ibnet {
namespace con {

JobManager::JobManager() :
    ThreadLoop("JobManager"),
    m_queue(1024),
    m_idleJob(nullptr),
    m_jobIdTypeCounter(0),
    m_dispatcherLock(),
    m_dispatcher()
{
    Start();
}

JobManager::~JobManager()
{
    Stop();

    delete m_idleJob;
}

JobQueue::JobType JobManager::GenerateJobTypeId()
{
    if (m_jobIdTypeCounter == JobQueue::JOB_TYPE_INVALID) {
        throw sys::IllegalStateException("Out of job type ids");
    }

    std::lock_guard<std::mutex> l(m_dispatcherLock);

    JobQueue::JobType type = m_jobIdTypeCounter++;

    m_dispatcher.resize(m_jobIdTypeCounter);

    return type;
}

void JobManager::AddJob(JobQueue::Job* job)
{
    IBNET_LOG_TRACE("Add job %d", job->m_type);

    while (!m_queue.PushBack(job)) {
        IBNET_LOG_WARN("Job queue full, waiting...");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // thread may be sleeping, wake up
    std::unique_lock<std::mutex> lock(m_jobLock);
    m_jobCondition.notify_all();
}

bool JobManager::IsQueueEmpty()
{
    return m_queue.IsEmpty();
}

void JobManager::_RunLoop()
{
    JobQueue::Job* job;

    job = m_queue.PopFront();

    // run discovery if no other jobs are available, prioritizing connection
    // creation and avoiding that discovery jobs are prioritized and cause
    // connection creation timeouts
    if (!job) {
        m_idleJobLock.lock();

        if (m_idleJob) {
            IBNET_LOG_DEBUG("Dispatching idle job type %d", m_idleJob->m_type);

            std::lock_guard<std::mutex> l(m_dispatcherLock);

            for (auto& it : m_dispatcher[m_idleJob->m_type]) {
                it->_DispatchJob(m_idleJob);
            }

            // re-use idle job, don't delete
        }

        m_idleJobLock.unlock();

        std::unique_lock<std::mutex> l(m_jobLock);

        if (m_queue.IsEmpty()) {
            // reduce CPU load and get woken up if a new job is available
            m_jobCondition.wait_for(l, std::chrono::milliseconds(1000));
        }
    } else {
        IBNET_LOG_DEBUG("Dispatching job type %d", job->m_type);

        m_dispatcherLock.lock();

        for (auto& it : m_dispatcher[job->m_type]) {
            it->_DispatchJob(job);
        }

        m_dispatcherLock.unlock();

        delete job;
    }
}

}
}