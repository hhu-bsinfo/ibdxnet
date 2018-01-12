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

#ifndef IBNET_DX_RECVTHREAD_H
#define IBNET_DX_RECVTHREAD_H

#include <atomic>
#include <memory>
#include <mutex>

#include "ibnet/core/IbCompQueue.h"
#include "ibnet/core/IbConnectionManager.h"
#include "ibnet/core/IbMemReg.h"
#include "ibnet/sys/ProfileTimer.hpp"
#include "ibnet/sys/ThreadLoop.h"
#include "ibnet/sys/Timer.hpp"

#include "RecvBufferPool.h"
#include "RecvHandler.h"

namespace ibnet {
namespace dx {

/**
 * Dedicated thread for receiving data.
 *
 * The thread is polling the buffer and flow control data (shared) receive
 * queues. It adds new work requests once old ones are consumed. The received
 * data is forwarded to handlers in the jvm space.
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.06.2017
 */
class RecvThread : public sys::ThreadLoop
{
public:
    /**
     * Constructor

     * @param connectionManager The parent connection manager
     * @param sharedRecvCQ Shared receive buffer queue to use
     * @param recvBufferPool Buffer pool to use to fill up the
     *          queues with new work requests
     * @param recvHandler Handler which forwards the received data
     */
    RecvThread(
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<RecvBufferPool>& recvBufferPool,
        std::shared_ptr<RecvHandler>& recvHandler);

    /**
     * Destructor
     */
    ~RecvThread(void);

    /**
     * Notify the receiver thread about a new connection created
     *
     * @param connection New connection established
     */
    void NodeConnected(core::IbConnection& connection);

protected:
    void _RunLoop(void) override;

private:
    std::shared_ptr<core::IbConnectionManager> m_connectionManager;
    std::shared_ptr<core::IbCompQueue> m_sharedRecvCQ;
    std::shared_ptr<RecvBufferPool> m_recvBufferPool;
    std::shared_ptr<RecvHandler> m_recvHandler;

private:
    std::atomic<bool> m_sharedQueueInitialFill;
    uint64_t m_recvBytes;
    sys::Timer m_waitTimer;

};

}
}

#endif //IBNET_DX_RECVTHREAD_H
