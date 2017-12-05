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

#ifndef IBNET_DX_RECVHANDLER_H
#define IBNET_DX_RECVHANDLER_H

#include "ibnet/core/IbMemReg.h"

#include "JNIHelper.h"

namespace ibnet {
namespace dx {

/**
 * Handle received buffers and pass them into the jvm space
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 07.07.2017
 */
class RecvHandler
{
public:
    /**
     * Constructor
     *
     * @param env The java environment from a java thread
     * @param object Java object of the equivalent callback class in java
     */
    RecvHandler(JNIEnv* env, jobject object);

    /**
     * Destructor
     */
    ~RecvHandler(void);

    /**
     * Called when a new fc and/or data buffer was received
     *
     * @param fcSource Node id of the FC data source
     * @param fcData FC data (or 0 if none)
     * @param dataSource The source node id of the data
     * @param dataMem Pointer to the IbMemReg object of the buffer or null if
     *                no data available
     * @param dataBuffer Pointer to the buffer with the received data
     * @param dataLength Number of data bytes received
     */
    inline void Received(uint16_t fcSource, uint32_t fcData,
            uint16_t dataSource, core::IbMemReg* dataMem, void* dataBuffer,
            uint32_t dataLength)
    {
        IBNET_LOG_TRACE_FUNC;

        JNIEnv* env = JNIHelper::GetEnv(m_vm);

        env->CallVoidMethod(m_object, m_midReceived, fcSource, fcData,
            dataSource, (uintptr_t) dataMem, dataBuffer, dataLength);

        JNIHelper::ReturnEnv(m_vm, env);

        IBNET_LOG_TRACE_FUNC_EXIT;
    }

private:
    JavaVM* m_vm;
    jobject m_object;

    jmethodID m_midReceived;

    jfieldID m_directBufferAddressField;
};

}
}

#endif //IBNET_DX_RECVHANDLER_H
