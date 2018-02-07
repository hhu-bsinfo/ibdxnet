//
// Created by on 1/30/18.
//

#include "ExecutionEngine.h"

#include "ibnet/sys/IllegalStateException.h"

namespace ibnet {
namespace dx {

ExecutionEngine::ExecutionEngine(uint16_t threadCount,
        stats::StatisticsManager* refStatisticsManager) :
    m_refStatisticsManager(refStatisticsManager)
{
    for (uint16_t i = 0; i < threadCount; i++) {
        m_workers.push_back(new Worker(i, refStatisticsManager));
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

ExecutionEngine::Worker::Worker(uint16_t id,
        stats::StatisticsManager* refStatisticsManager) :
    ThreadLoop("ExecutionEngineWorker-" + std::to_string(id)),
    m_id(id),
    m_refStatisticsManager(refStatisticsManager),
    m_executionUnits(),
    m_idleTimer(),
    m_idleCounter(
        new stats::Unit("EE-Worker-" + std::to_string(id) + "-Idle")),
    m_activeCounter(
        new stats::Unit("EE-Worker-" + std::to_string(id) + "-Active")),
    m_yieldCounter(
        new stats::Unit("EE-Worker-" + std::to_string(id) + "-Yield")),
    m_sleepCounter(
        new stats::Unit("EE-Worker-" + std::to_string(id) + "-Sleep")),
    m_activityRatio(new stats::Ratio(
        "EE-Worker-" + std::to_string(id) + "-ActivityRatio", m_activeCounter,
        m_idleCounter))
{
    m_refStatisticsManager->Register(m_idleCounter);
    m_refStatisticsManager->Register(m_activeCounter);
    m_refStatisticsManager->Register(m_yieldCounter);
    m_refStatisticsManager->Register(m_sleepCounter);
    m_refStatisticsManager->Register(m_activityRatio);
}

ExecutionEngine::Worker::~Worker()
{
    m_refStatisticsManager->Deregister(m_idleCounter);
    m_refStatisticsManager->Deregister(m_activeCounter);
    m_refStatisticsManager->Deregister(m_yieldCounter);
    m_refStatisticsManager->Deregister(m_sleepCounter);
    m_refStatisticsManager->Deregister(m_activityRatio);

    delete m_idleCounter;
    delete m_activeCounter;
    delete m_yieldCounter;
    delete m_sleepCounter;
    delete m_activityRatio;
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

        if (m_idleTimer.GetTimeMs() > 1000.0) {
            m_sleepCounter->Inc();
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        } else if (m_idleTimer.GetTimeMs() > 100.0) {
            m_yieldCounter->Inc();
            std::this_thread::yield();
        }

        m_idleCounter->Inc();
    } else {
        m_idleTimer.Stop();

        m_activeCounter->Inc();
    }
}

}
}