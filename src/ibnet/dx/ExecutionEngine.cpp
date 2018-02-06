//
// Created by on 1/30/18.
//

#include "ExecutionEngine.h"

#include "ibnet/sys/IllegalStateException.h"

namespace ibnet {
namespace dx {

ExecutionEngine::ExecutionEngine(uint16_t threadCount)
{
    for (uint16_t i = 0; i < threadCount; i++) {
        m_workers.push_back(new Worker(i));
    }
}

ExecutionEngine::~ExecutionEngine()
{
    for (auto& it : m_workers) {
        delete it;
    }
}

void ExecutionEngine::PinWorker(uint16_t workerId, uint16_t cpuid)
{
    if (workerId >= m_workers.size()) {
        throw sys::IllegalStateException("Worker id invalid: %d", workerId);
    }

    m_workers[workerId]->Pin(cpuid);
}

void ExecutionEngine::AddExecutionUnit(uint16_t workerId,
        ExecutionUnit* executionUnit)
{
    if (workerId >= m_workers.size()) {
        throw sys::IllegalStateException("Worker id invalid: %d", workerId);
    }

    m_workers[workerId]->AddExecutionUnit(executionUnit);
}

void ExecutionEngine::Start()
{
    IBNET_LOG_INFO("Starting execution engine with %d workers",
        m_workers.size());

    for (auto& it : m_workers) {
        it->Start();
    }

    IBNET_LOG_DEBUG("Execution engine started");
}

void ExecutionEngine::Stop()
{
    IBNET_LOG_INFO("Stopping execution engine...");

    for (auto& it : m_workers) {
        it->Stop();
    }

    IBNET_LOG_DEBUG("Execution engine stopped");
}

ExecutionEngine::Worker::Worker(uint16_t id) :
    ThreadLoop("ExecutionEngineWorker-" + std::to_string(id)),
    m_id(id),
    m_executionUnits(),
    m_idleTimer(),
    m_idleCount(0)
{

}

void ExecutionEngine::Worker::AddExecutionUnit(ExecutionUnit* executionUnit)
{
    if (IsStarted()) {
        throw sys::IllegalStateException("EE already running");
    }

    m_executionUnits.push_back(executionUnit);
}

void ExecutionEngine::Worker::_BeforeRunLoop()
{
    std::string schedule;

    for (auto& it : m_executionUnits) {
        schedule += it->GetName() + ", ";
    }

    IBNET_LOG_INFO("Worker %d, executing unit schedule: %s", m_id, schedule);
}

void ExecutionEngine::Worker::_RunLoop()
{
    bool idle = true;

    for (auto& it : m_executionUnits) {
        if (it->Dispatch()) {
            idle = false;
        }
    }

    // execute parking strategy only if all units are idle
    if (idle) {
        if (!m_idleTimer.IsRunning()) {
            m_idleTimer.Start();
        }

        if (m_idleTimer.GetTimeMs() > 100.0) {
            std::this_thread::yield();
        } else if (m_idleTimer.GetTimeMs() > 1000.0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }

        m_idleCount++;
    } else {
        m_idleTimer.Stop();
    }
}

}
}