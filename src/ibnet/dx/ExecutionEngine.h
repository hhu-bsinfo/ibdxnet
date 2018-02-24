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

#ifndef IBNET_DX_EXECUTIONENGINE_H
#define IBNET_DX_EXECUTIONENGINE_H

#include "ibnet/sys/ThreadLoop.h"
#include "ibnet/sys/Timer.hpp"

#include "ibnet/stats/Ratio.hpp"
#include "ibnet/stats/StatisticsManager.h"
#include "ibnet/stats/Unit.hpp"

#include "ExecutionUnit.h"

namespace ibnet {
namespace dx {

/**
 * Engine executing (execution) units with a specified number of workers. The EUs
 * are explicitly assigned to a specific worker and not scheduled dynamically.
 * This simply supports creating static execution flows more easily instead of
 * hardcoding them.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 30.01.2018
 */
class ExecutionEngine
{
public:
    /**
     * Constructor
     *
     * @param threadCount Number of worker threads to spawn
     * @param refStatisticsManager Pointer to statistics manager (caller has to manage)
     */
    explicit ExecutionEngine(uint16_t threadCount,
        stats::StatisticsManager* refStatisticsManager);

    /**
     * Destructor
     */
    ~ExecutionEngine();

    /**
     * Pin a specific worker to a single virtual/physical CPU core
     *
     * @param workerId Worker to pin
     * @param cpuid Target cpuid of the core
     */
    void PinWorker(uint16_t workerId, uint16_t cpuid);

    /**
     * Add a execution unit to a worker. The EUs are executed in the order
     * they are added and repeated in the same order
     *
     * @param workerId Target worker id
     * @param executionUnit Execution unit to add (caller has to manage memory)
     */
    void AddExecutionUnit(uint16_t workerId, ExecutionUnit* refExecutionUnit);

    /**
     * Start the engine
     */
    void Start();

    /**
     * Stop the engine
     */
    void Stop();

private:
    /**
     * Worker thread of the engine
     */
    class Worker : public sys::ThreadLoop
    {
    public:
        explicit Worker(uint16_t id,
            stats::StatisticsManager* refStatisticsManager);

        ~Worker() override;

        void AddExecutionUnit(ExecutionUnit* executionUnit);

    protected:
        void _BeforeRunLoop() override;

        void _RunLoop() override;

    private:
        const uint16_t m_id;
        stats::StatisticsManager* m_refStatisticsManager;

        std::vector<ExecutionUnit*> m_executionUnits;

    private:
        sys::Timer m_idleTimer;

        stats::Unit* m_idleCounter;
        stats::Unit* m_activeCounter;
        stats::Unit* m_yieldCounter;
        stats::Unit* m_sleepCounter;
        stats::Ratio* m_activityRatio;
    };

    stats::StatisticsManager* m_refStatisticsManager;

    std::vector<Worker*> m_workers;
};

}
}

#endif //IBNET_DX_EXECUTIONENGINE_H
