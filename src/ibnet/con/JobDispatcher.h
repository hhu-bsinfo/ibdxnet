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

#ifndef IBNET_CON_JOBDISPATCHER_H
#define IBNET_CON_JOBDISPATCHER_H

#include "JobQueue.h"

namespace ibnet {
namespace con {

// forward declaration friendship
class JobManager;

/**
 * Interface for a dispatcher handling a job that is executed
 * by the JobManager
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 29.01.2018
 */
class JobDispatcher
{
public:
    friend JobManager;

protected:
    /**
     * Constructor
     */
    JobDispatcher() = default;

    /**
     * Destructor
     */
    virtual ~JobDispatcher() = default;

    /**
     * Dispatcher method called by manager if a target job is
     * executed.
     *
     * @param job Job to dispatch (caller is managing memory)
     */
    virtual void _DispatchJob(const JobQueue::Job* job) = 0;
};

}
}

#endif //IBNET_CON_JOBDISPATCHER_H
