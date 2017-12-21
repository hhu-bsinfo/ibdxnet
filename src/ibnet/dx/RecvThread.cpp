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

    // sanity check
    if (!connection.GetQp(1)->GetRecvQueue()->IsRecvQueueShared()) {
        throw DxnetException("Can't work with non shared FC recv queue(s)");
    }

    size = connection.GetQp(1)->GetRecvQueue()->GetQueueSize();
    for (uint32_t i = 0; i < size; i++) {
        core::IbMemReg* buf = m_recvBufferPool->GetFlowControlBuffer();

        // Use the pointer as the work req id
        connection.GetQp(1)->GetRecvQueue()->Receive(buf, (uint64_t) buf);
    }
}

void RecvThread::_RunLoop(void)
{
    uint32_t fcData;
    uint16_t fcSourceNodeId;

    core::IbMemReg* dataMem;
    void* data;
    uint16_t dataSourceNodeId;
    uint32_t dataRecvLength;

    fcData = __ProcessFlowControl(&fcSourceNodeId);
    dataMem = __ProcessBuffers(&dataSourceNodeId, &dataRecvLength);

    if (dataMem) {
        data = dataMem->GetAddress();
    } else {
        data = nullptr;
    }

    if (!fcData && !dataMem) {
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
        m_recvHandler->Received(fcSourceNodeId, fcData, dataSourceNodeId,
            dataMem, data, dataRecvLength);
    }
}

uint32_t RecvThread::__ProcessFlowControl(uint16_t* sourceNodeId)
{
    uint64_t workReqId = (uint64_t) -1;
    uint32_t recvLength = 0;
    uint32_t flowControlData;
    *sourceNodeId = static_cast<uint16_t>(-1);

    try {
        *sourceNodeId = m_sharedFlowControlRecvCQ->PollForCompletion(false,
            &workReqId, &recvLength);
    } catch (core::IbException& e) {
        IBNET_LOG_ERROR("Polling for data buffer completion failed: {}",
            e.what());
        return 0;
    }

    // no flow control data available
    if (*sourceNodeId == core::IbNodeId::INVALID) {
        return 0;
    }

    auto mem = (core::IbMemReg*) workReqId;
    m_recvFlowControlBytes += recvLength;
    flowControlData = *((uint32_t*) mem->GetAddress());

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(*sourceNodeId);

    // keep the recv queue filled, using a shared recv queue here
    connection->GetQp(1)->GetRecvQueue()->Receive(mem, (uint64_t) mem);

    m_connectionManager->ReturnConnection(connection);

    return flowControlData;
}

core::IbMemReg* RecvThread::__ProcessBuffers(uint16_t* sourceNodeId,
        uint32_t* recvLength)
{
    uint64_t workReqId = (uint64_t) -1;
    *sourceNodeId = static_cast<uint16_t>(-1);
    *recvLength = 0;

    try {
        *sourceNodeId = m_sharedRecvCQ->PollForCompletion(false, &workReqId,
            recvLength);
    } catch (core::IbException& e) {
        IBNET_LOG_ERROR("Polling for flow control completion failed: {}",
            e.what());
        return nullptr;
    }

    // no data available
    if (*sourceNodeId == core::IbNodeId::INVALID) {
        return nullptr;
    }

    auto mem = (core::IbMemReg*) workReqId;
    m_recvBytes += *recvLength;

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(*sourceNodeId);

    // keep the recv queue filled, using a shared recv queue here
    // get another buffer from the pool
    core::IbMemReg* buf = m_recvBufferPool->GetBuffer();

    // Use the pointer as the work req id
    connection->GetQp(0)->GetRecvQueue()->Receive(buf,
        (uint64_t) buf);

    m_connectionManager->ReturnConnection(connection);

    // buffer is return to the pool async
    return mem;
}

}
}