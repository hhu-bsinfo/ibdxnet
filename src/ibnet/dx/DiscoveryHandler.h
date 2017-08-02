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

#ifndef IBNET_DX_DISCOVERYHANDLER_H
#define IBNET_DX_DISCOVERYHANDLER_H

#include "ibnet/core/IbDiscoveryManager.h"

#include "JNIHelper.h"

namespace ibnet {
namespace dx {

/**
 * Handle node discovered/invalidated events by calling back to the jvm space
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.07.2017
 */
class DiscoveryHandler : public ibnet::core::IbDiscoveryManager::Listener
{
public:
    /**
     * Constructor
     *
     * @param env The java environment from a java thread
     * @param object Java object of the equivalent callback class in java
     */
    DiscoveryHandler(JNIEnv* env, jobject object);

    /**
     * Destructor
     */
    ~DiscoveryHandler(void);

    /**
     * Override
     */
    void NodeDiscovered(uint16_t nodeId) override
    {
        __NodeDiscovered(nodeId);
    }

    /**
     * Override
     */
    void NodeInvalidated(uint16_t nodeId) override
    {
        __NodeInvalidated(nodeId);
    }

private:
    inline void __NodeDiscovered(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeDiscovered, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

    inline void __NodeInvalidated(uint16_t nodeId)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);
        env->CallVoidMethod(m_object, m_midNodeInvalidated, nodeId);
        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    jmethodID m_midNodeDiscovered;
    jmethodID m_midNodeInvalidated;
};

}
}

#endif //IBNET_DX_DISCOVERYHANDLER_H
