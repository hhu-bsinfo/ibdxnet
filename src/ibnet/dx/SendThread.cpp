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
    m_sentFlowControlConfirms(0)
{

}

SendThread::~SendThread(void)
{

}

void SendThread::_BeforeRunLoop(void)
{
    // TODO make configurable
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset)) {
        IBNET_LOG_ERROR("Setting cpu affinity failed");
    }
}

void SendThread::_RunLoop(void)
{
    SendHandler::NextWorkParameters* data = m_sendHandler->GetNextDataToSend(
        m_prevNodeIdWritten, m_prevDataWritten);

    // reset previous state
    m_prevNodeIdWritten = core::IbNodeId::INVALID;
    m_prevDataWritten = 0;

    // if nothing to process, thread should wait in java space but might
    // return sometimes to allow this thread to join on shutdown
    if (data == nullptr) {
        return;
    }

    // seems like we got something to process
    m_prevNodeIdWritten = data->m_nodeId;

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(data->m_nodeId);

    // connection closed in the meantime
    if (!connection) {
        // sent back to java space on the next GetNext call
        return;
    }

    try {
        uint32_t totalBytesSent = 0;

        // we are expecting the ring buffer (in java) to handle overflows
        // and slice them correctly, i.e. posFrontRel <= posBackRel, always
        // sanity check that
        if (data->m_posFrontRel > data->m_posBackRel) {
            IBNET_LOG_PANIC("posFrontRel {} > posBackRel {} not allowed",
                data->m_posFrontRel, data->m_posBackRel);
            return;
        }

        core::IbMemReg* sendBuffer =
            m_buffers->GetBuffer(connection->GetConnectionId());

        // set first bit to indicate FC confirmation
        uint16_t immedData =
            static_cast<uint16_t>(data->m_flowControlData > 0 ? 1 : 0);

        uint16_t queueSize = connection->GetQp(0)->GetSendQueue()->GetQueueSize();
        uint32_t posFront = data->m_posFrontRel;
        uint32_t posBack = data->m_posBackRel;

        while (posFront != posBack || immedData) {
            uint16_t sliceCount = 0;
            uint32_t iterationBytesSent = 0;

            // slice area of send buffer into slices fitting receive buffers
            while (sliceCount < queueSize && posFront != posBack || immedData) {
                // fits a full receive buffer
                if (posFront + m_recvBufferSize <= posBack) {
                    connection->GetQp(0)->GetSendQueue()->Send(sendBuffer,
                        immedData, posFront, m_recvBufferSize);

                    posFront += m_recvBufferSize;
                    iterationBytesSent += m_recvBufferSize;
                } else {
                    // smaller than a receive buffer
                    uint32_t size = posBack - posFront;
                    bool zeroLength = false;

                    // don't send packages with size 0 which is a special value
                    // and gets translated to 2^31 bytes length
                    // use a flag to indicate 0 length payloads
                    if (size == 0) {
                        immedData |= (1 << 1);
                        // send some dummy data which is ignored on incoming
                        size = 1;
                        zeroLength = true;
                    }

                    connection->GetQp(0)->GetSendQueue()->Send(sendBuffer,
                        immedData, posFront, size);

                    if (!zeroLength) {
                        posFront += size;
                        iterationBytesSent += size;
                    }
                }

                // send fc confirmation once and clear when done
                if (immedData) {
                    immedData = 0;
                    m_sentFlowControlConfirms++;
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

        m_connectionManager->ReturnConnection(connection);

        m_prevDataWritten = totalBytesSent;
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

}
}