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

#ifndef IBNET_DX_RECVBUFFERPOOL_H
#define IBNET_DX_RECVBUFFERPOOL_H

#include <atomic>

#include "ibnet/core/IbProtDom.h"

namespace ibnet {
namespace dx {

/**
 * Buffer pool with buffers registered with a protection domain
 * for incoming data. Implements a lock free 1:N (consumer:producer)
 * ring buffer.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class RecvBufferPool
{
public:
    /**
     * Constructor
     *
     * @param totalPoolSize Total size of the pool in bytes
     * @param recvBufferSize Size of a single receive buffer in the pool
     * @param protDom Protection domain to register all buffers at
     */
    RecvBufferPool(uint64_t totalPoolSize, uint32_t recvBufferSize,
        core::IbProtDom* refProtDom);

    /**
     * Destructor
     */
    ~RecvBufferPool();

    /**
     * Get a buffer from the pool. If the pool is empty, no new buffers are
     * allocated and the caller is waiting actively until a buffer is returned.
     *
     * @return Buffer
     */
    core::IbMemReg* GetBuffer();

    uint32_t GetBuffers(core::IbMemReg** retBuffers, uint32_t count);

    /**
     * Return a buffer to be reused
     *
     * @param buffer Buffer to return
     */
    void ReturnBuffer(core::IbMemReg* buffer);

    void ReturnBuffers(core::IbMemReg** buffers, uint32_t count);

private:
    const uint64_t m_bufferPoolSize;
    const uint32_t m_bufferSize;

    core::IbProtDom* m_refProtDom;

    std::atomic<uint32_t> m_dataBuffersFront;
    std::atomic<uint32_t> m_dataBuffersBack;
    std::atomic<uint32_t> m_dataBuffersBackRes;

    core::IbMemReg* m_memoryPool;
    core::IbMemReg** m_dataBuffers;

    std::atomic<uint64_t> m_insufficientBufferCounter;
};

}
}

#endif //IBNET_DX_RECVBUFFERPOOL_H
