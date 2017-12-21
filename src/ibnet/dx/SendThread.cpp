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

#include "SendThread.h"

#include "ibnet/core/IbDisconnectedException.h"

namespace ibnet {
namespace dx {

SendThread::SendThread(uint32_t recvBufferSize,
        std::shared_ptr<SendBuffers> buffers,
        std::shared_ptr<SendHandler>& sendHandler,
        std::shared_ptr<core::IbConnectionManager>& connectionManager) :
    ThreadLoop("SendThread"),
    m_recvBufferSize(recvBufferSize),
    m_buffers(buffers),
    m_sendHandler(sendHandler),
    m_connectionManager(connectionManager),
    m_prevNodeIdWritten(core::IbNodeId::INVALID),
    m_prevDataWritten(0),
    m_sentBytes(0),
    m_sentFlowControlBytes(0),
    m_waitTimer()
{

}

SendThread::~SendThread(void)
{

}

void SendThread::_RunLoop(void)
{
    SendHandler::NextWorkParameters* data = m_sendHandler->GetNextDataToSend(
        m_prevNodeIdWritten, m_prevDataWritten);

    // reset previous state
    m_prevNodeIdWritten = core::IbNodeId::INVALID;
    m_prevDataWritten = 0;

    // nothing to process
    if (data == nullptr) {
        if (!m_waitTimer.IsRunning()) {
            m_waitTimer.Start();
        }

        if (m_waitTimer.GetTimeMs() > 100.0) {
            std::this_thread::yield();
        } else if (m_waitTimer.GetTimeMs() > 1000.0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }

        return;
    }

    m_waitTimer.Stop();

    // seems like we got something to process
    m_prevNodeIdWritten = data->m_nodeId;

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(data->m_nodeId);

    // connection closed in the meantime
    if (!connection) {
        // sent back to java space on the next GetNext call
        m_prevDataWritten = 0;
        return;
    }

    try {
        __ProcessFlowControl(connection, data);
        m_prevDataWritten = __ProcessBuffer(connection, data);

        m_connectionManager->ReturnConnection(connection);
    } catch (core::IbQueueClosedException& e) {
        m_connectionManager->ReturnConnection(connection);
        // ignore
    } catch (core::IbDisconnectedException& e) {
        IBNET_LOG_DEBUG("DisconnectedException on node {}",
            connection->GetRemoteNodeId());

        m_connectionManager->ReturnConnection(connection);
        m_connectionManager->CloseConnection(connection->GetRemoteNodeId(),
            true);
    }
}

uint32_t SendThread::__ProcessFlowControl(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data)
{
    const uint32_t numBytesToSend = sizeof(uint32_t);

    if (data->m_flowControlData == 0) {
        return 0;
    }

    core::IbMemReg* mem = m_buffers->GetFlowControlBuffer(
        connection->GetConnectionId());

    memcpy(mem->GetAddress(), &data->m_flowControlData, numBytesToSend);

    connection->GetQp(1)->GetSendQueue()->Send(mem, 0, numBytesToSend);

    connection->GetQp(1)->GetSendQueue()->PollCompletion(true);

    m_sentFlowControlBytes += numBytesToSend;

    return data->m_flowControlData;
}

uint32_t SendThread::__ProcessBuffer(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data)
{
    uint32_t totalBytesSent = 0;

    // we are expecting the ring buffer (in java) to handle overflows
    // and slice them correctly, i.e. posFrontRel <= posBackRel, always
    // sanity check that
    if (data->m_posFrontRel > data->m_posBackRel) {
        IBNET_LOG_PANIC("posFrontRel {} > posBackRel {} not allowed",
            data->m_posFrontRel, data->m_posBackRel);
        return 0;
    }

    // no data to send but FC data should be available
    if (data->m_posFrontRel == data->m_posBackRel) {
        return 0;
    }

    core::IbMemReg* sendBuffer =
        m_buffers->GetBuffer(connection->GetConnectionId());

    uint16_t queueSize = connection->GetQp(0)->GetSendQueue()->GetQueueSize();
    uint32_t posFront = data->m_posFrontRel;
    uint32_t posBack = data->m_posBackRel;

    while (posFront != posBack) {
        uint16_t sliceCount = 0;
        uint32_t iterationBytesSent = 0;

        // slice area of send buffer into slices fitting receive buffers
        while (sliceCount < queueSize && posFront != posBack) {
            // fits a full receive buffer
            if (posFront + m_recvBufferSize <= posBack) {
                connection->GetQp(0)->GetSendQueue()->Send(sendBuffer,
                    posFront, m_recvBufferSize);

                posFront += m_recvBufferSize;
                iterationBytesSent += m_recvBufferSize;
            } else {
                // smaller than a receive buffer
                uint32_t size = posBack - posFront;

                connection->GetQp(0)->GetSendQueue()->Send(sendBuffer,
                    posFront, size);

                posFront += size;
                iterationBytesSent += size;
            }

            sliceCount++;
        }

        // poll completions
        for (uint16_t i = 0; i < sliceCount; i++) {
            connection->GetQp(0)->GetSendQueue()->PollCompletion(true);
        }

        m_sentBytes += iterationBytesSent;
        totalBytesSent += iterationBytesSent;
    }

    return totalBytesSent;
}

}
}