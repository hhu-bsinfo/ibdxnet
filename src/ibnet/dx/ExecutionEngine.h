//
// Created by on 1/30/18.
//

#ifndef IBNET_DX_EXECUTIONENGINE_H
#define IBNET_DX_EXECUTIONENGINE_H

#include "ibnet/sys/ThreadLoop.h"
#include "ibnet/sys/Timer.hpp"

#include "ExecutionUnit.h"

namespace ibnet {
namespace dx {

class ExecutionEngine
{
public:
    explicit ExecutionEngine(uint16_t threadCount);

    ~ExecutionEngine();

    void PinWorker(uint16_t workerId, uint16_t cpuid);

    void AddExecutionUnit(uint16_t workerId, ExecutionUnit* executionUnit);

    void Start();

    void Stop();

private:
    class Worker : public sys::ThreadLoop
    {
    public:
        explicit Worker(uint16_t id);

        ~Worker() override = default;

        void AddExecutionUnit(ExecutionUnit* executionUnit);

    protected:
        void _BeforeRunLoop() override;

        void _RunLoop() override;

    private:
        const uint16_t m_id;
        std::vector<ExecutionUnit*> m_executionUnits;

    private:
        sys::Timer m_idleTimer;
        uint64_t m_idleCount;
    };

    std::vector<Worker*> m_workers;
};

}
}

#endif //IBNET_DX_EXECUTIONENGINE_H