/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
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

#ifndef IBNET_SYS_THREAD_H
#define IBNET_SYS_THREAD_H

#include <chrono>
#include <thread>

#include "Exception.h"
#include "Logger.hpp"
#include "SystemException.h"

namespace ibnet {
namespace sys {

/**
 * Thread base class based on std::thread with additional features
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Thread
{
public:
    /**
     * Constructor
     *
     * @param name Name of the thread (for debugging)
     */
    explicit Thread(const std::string& name = "") :
        m_name(name),
        m_thread(nullptr)
    {}

    /**
     * Destructor
     */
    virtual ~Thread() = default;

    /**
     * Get the thread name
     */
    const std::string& GetName() const {
        return m_name;
    }

    /**
     * Check if the thread is started
     *
     * @return True if started, false otherwise
     */
    bool IsStarted() const {
        return m_thread != nullptr;
    }

    /**
     * Start the thread. A thread can be restarted once it finished execution
     */
    void Start() {
        m_thread = new std::thread(&Thread::__Run, this);
    }

    /**
     * Join the started thread
     */
    void Join()
    {
        m_thread->join();
        delete m_thread;
    }

    /**
     * Pin the thread to be executed on a specific physical core, only
     *
     * @param cpuid Id of the cpu core to pin to
     */
    void Pin(uint16_t cpuid)
    {
        cpu_set_t cpuset = {};
        CPU_ZERO(&cpuset);
        CPU_SET(cpuid, &cpuset);

        pthread_t current_thread = pthread_self();
        if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t),
                &cpuset)) {
            sys::SystemException("Setting cpu affinity for %s to %d failed",
                m_name, cpuid);
        }
    }

protected:
    /**
     * Method executed by new thread. Implement this
     */
    virtual void _Run() = 0;

    /**
     * Yield this thread
     */
    void _Yield() {
        std::this_thread::yield();
    }

    /**
     * Put thread to sleep
     * @param timeMs Number of ms to sleep
     */
    void _Sleep(uint32_t timeMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
    }

private:
    const std::string m_name;
    std::thread* m_thread;

    void __Run()
    {
        try {
            IBNET_LOG_INFO("Started thread %s", m_name);
            _Run();
            IBNET_LOG_INFO("Finished thread %s", m_name);
        } catch (Exception& e) {
            e.PrintStackTrace();
            throw e;
        }
    }
};

}
}

#endif //IBNET_SYS_THREAD_H
