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
        std::shared_ptr<RecvBufferPool>& recvBufferPool,
        std::shared_ptr<RecvHandler>& recvHandler) :
    ThreadLoop("RecvThread"),
    m_connectionManager(connectionManager),
    m_sharedRecvCQ(sharedRecvCQ),
    m_recvBufferPool(recvBufferPool),
    m_recvHandler(recvHandler),
    m_sharedQueueInitialFill(false),
    m_recvBytes(0),
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

void RecvThread::_BeforeRunLoop(void)
{
    // TODO make configurable
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset)) {
        IBNET_LOG_ERROR("Setting cpu affinity failed");
    }
}

void RecvThread::_RunLoop(void)
{
    uint16_t sourceNodeId = static_cast<uint16_t>(-1);
    bool fcConfirm = false;
    core::IbMemReg* dataMem = nullptr;
    void* data = nullptr;
    uint32_t dataRecvLength = 0;

    while (true) {
        uint64_t workReqId = (uint64_t) -1;
        uint16_t immedData = 0;
        bool zeroLengthPackage = false;

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

        // eval immediate data containing flow control confirmation
        if (immedData & (1 << 0)) {
            fcConfirm = true;
        }

        // check for zero length package which can't be indicated by setting
        // the size to 0 on send (which gets translated to 2^31 instead)
        if (immedData & (1 << 1)) {
            zeroLengthPackage = true;
        }

        if (!zeroLengthPackage) {
            // pointer to memory with received data is stored os the work req id
            dataMem = (core::IbMemReg*) workReqId;
            m_recvBytes += dataRecvLength;

            // buffer is return to the pool async from java space
            data = dataMem->GetAddress();
        } else {
            // set length to actual value
            dataRecvLength = 0;

            // return buffer to pool, don't care about any dummy data
            // otherwise, the pool runs dry after a while
            // pointer to memory with received data is stored os the work req id
            m_recvBufferPool->ReturnBuffer((core::IbMemReg*) workReqId);
        }

        std::shared_ptr < core::IbConnection > connection =
            m_connectionManager->GetConnection(sourceNodeId);

        // keep the recv queue filled, using a shared recv queue here
        // get another buffer from the pool
        core::IbMemReg* newRecvMem = m_recvBufferPool->GetBuffer();

        // Use the pointer as the work req id
        connection->GetQp(0)->GetRecvQueue()->Receive(newRecvMem,
            (uint64_t) newRecvMem);

        m_connectionManager->ReturnConnection(connection);

        break;
    }

    if (!data && !fcConfirm) {
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