//
// Created by nothaas on 1/29/18.
//

#ifndef IBNET_CON_JOBTHREAD_H
#define IBNET_CON_JOBTHREAD_H

#include <condition_variable>
#include "ibnet/sys/ThreadLoop.h"

#include "JobDispatcher.h"
#include "JobQueue.h"

namespace ibnet {
namespace con {

class JobManager : public sys::ThreadLoop
{
public:
    JobManager();

    ~JobManager() override;

    JobQueue::JobType GenerateJobTypeId();

    void AddDispatcher(JobQueue::JobType type, JobDispatcher* dispatcher) {
        std::lock_guard<std::mutex> l(m_dispatcherLock);
        m_dispatcher[type].push_back(dispatcher);
    }

    void RemoveDispatcher(JobQueue::JobType type, JobDispatcher* dispatcher) {
        std::lock_guard<std::mutex> l(m_dispatcherLock);

        for (auto it = m_dispatcher[type].begin();
             it != m_dispatcher[type].end(); it++) {
            if (*it == dispatcher) {
                m_dispatcher[type].erase(it);
                break;
            }
        }
    }

    void AddJob(JobQueue::Job* job);

    // job to execute when queue is empty
    void SetIdleJob(JobQueue::Job* job) {
        std::lock_guard<std::mutex> l(m_idleJobLock);
        delete m_idleJob;
        m_idleJob = job;
    }

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
