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

#include "RecvThread.h"

#include "ibnet/core/IbDisconnectedException.h"

#include "DxnetException.h"

namespace ibnet {
namespace dx {

RecvThread::RecvThread(
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<core::IbCompQueue>& sharedFlowControlRecvCQ,
        std::shared_ptr<RecvBufferPool>& recvBufferPool,
        std::shared_ptr<RecvHandler>& recvHandler) :
    ThreadLoop("RecvThread"),
    m_connectionManager(connectionManager),
    m_sharedRecvCQ(sharedRecvCQ),
    m_sharedFlowControlRecvCQ(sharedFlowControlRecvCQ),
    m_recvBufferPool(recvBufferPool),
    m_recvHandler(recvHandler),
    m_sharedQueueInitialFill(false),
    m_recvBytes(0),
    m_recvFlowControlBytes(0),
    m_waitTimer()
{

}

RecvThread::~RecvThread(void)
{

}

void RecvThread::NodeConnected(core::IbConnection& connection)
{
    // on the first connection, fill the shared recv queue
    bool expected = false;
    if (!m_sharedQueueInitialFill.compare_exchange_strong(expected, true,
            std::memory_order_relaxed)) {
        return;
    }

    // sanity check
    if (!connection.GetQp(0)->GetRecvQueue()->IsRecvQueueShared()) {
        throw DxnetException("Can't work with non shared recv queue(s)");
    }

    uint32_t size = connection.GetQp(0)->GetRecvQueue()->GetQueueSize();
    for (uint32_t i = 0; i < size; i++) {
        core::IbMemReg* buf = m_recvBufferPool->GetBuffer();

        // Use the pointer as the work req id
        connection.GetQp(0)->GetRecvQueue()->Receive(buf, (uint64_t) buf);
    }
}

void RecvThread::_RunLoop(void)
{
    core::IbMemReg* dataMem = nullptr;
    void* data = nullptr;
    uint16_t sourceNodeId = static_cast<uint16_t>(-1);
    uint32_t dataRecvLength = 0;
    uint16_t immedData = 0;
    bool fcConfirm = false;

    while (true) {
        uint64_t workReqId = (uint64_t) -1;

        try {
            sourceNodeId = m_sharedRecvCQ->PollForCompletion(false,
                &workReqId, &dataRecvLength, &immedData);
        } catch (core::IbException &e) {
            IBNET_LOG_ERROR("Polling for flow control completion failed: {}",
                e.what());
            break;
        }

        // no data available
        if (sourceNodeId == core::IbNodeId::INVALID) {
            break;
        }

        printf(">>> received: %d\n", dataRecvLength);

        auto mem = (core::IbMemReg*) workReqId;
        m_recvBytes += dataRecvLength;

        std::shared_ptr < core::IbConnection > connection =
            m_connectionManager->GetConnection(sourceNodeId);

        // keep the recv queue filled, using a shared recv queue here
        // get another buffer from the pool
        core::IbMemReg* buf = m_recvBufferPool->GetBuffer();

        // Use the pointer as the work req id
        connection->GetQp(0)->GetRecvQueue()->Receive(buf,
            (uint64_t) buf);

        m_connectionManager->ReturnConnection(connection);

        // buffer is return to the pool async
        data = buf->GetAddress();

        // eval immediate data containing flow control confirmation
        if (immedData) {
            fcConfirm = true;
        }

        break;
    }

    if (!data) {
        if (!m_waitTimer.IsRunning()) {
            m_waitTimer.Start();
        }

        if (m_waitTimer.GetTimeMs() > 100.0) {
            std::this_thread::yield();
        } else if (m_waitTimer.GetTimeMs() > 1000.0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    } else {
        m_waitTimer.Stop();

        // pass data to jvm space
        m_recvHandler->Received(sourceNodeId, fcConfirm, dataMem, data,
            dataRecvLength);
    }
}

}
}