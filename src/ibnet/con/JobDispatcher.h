//
// Created by nothaas on 1/29/18.
//

#ifndef IBNET_CON_JOBDISPATCHER_H
#define IBNET_CON_JOBDISPATCHER_H

#include "JobQueue.h"

namespace ibnet {
namespace con {

// forward declaration friendship
class JobManager;

class JobDispatcher
{
public:
    friend JobManager;

protected:
    JobDispatcher() = default;

    virtual ~JobDispatcher() = default;

    virtual void _DispatchJob(const JobQueue::Job* const job) = 0;
};

}
}

#endif //IBNET_CON_JOBDISPATCHER_H
