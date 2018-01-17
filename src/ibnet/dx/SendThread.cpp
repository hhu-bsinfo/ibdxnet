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

#include "ibnet/core/IbQueueFullException.h"
#include "ibnet/core/IbQueuePair.h"

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
    m_sentFlowControlConfirms(0),
    m_sendCounter(0),
    m_pollCompCounter(0),
    // cache line alignment
    // TODO we need the queue size parameter here *20 hardcoded
    m_sgeLists(static_cast<ibv_sge*>(aligned_alloc(64, sizeof(ibv_sge) * 20))),
    m_sendWrs(static_cast<ibv_send_wr*>(aligned_alloc(64, sizeof(ibv_send_wr) * 20))),
    m_workComps(static_cast<ibv_wc*>(aligned_alloc(64, sizeof(ibv_wc) * 20))),
    m_firstWc(true)
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

         core::IbMemReg* sendBuffer =
            m_buffers->GetBuffer(connection->GetConnectionId());

        // set first bit to indicate FC confirmation
        uint16_t immedData =
            static_cast<uint16_t>(data->m_flowControlData > 0 ? 1 : 0);

        uint16_t queueSize = connection->GetQp(0)->GetSendQueue()->GetQueueSize();
        uint32_t posBack = data->m_posBackRel;
        uint32_t posFront = data->m_posFrontRel;




// TODO for debugging, remove later
//        uint32_t ssize = 0;
//        if (posBack < posFront) {
//            ssize = posFront - posBack;
//        } else {
//            ssize = sendBuffer->GetSize() - posBack + posFront;
//        }
//
//        IBNET_LOG_ERROR(">>> {} | {} | {} | {}", data->m_nodeId, ((double) ssize) / 1024 / 1024, data->m_posBackRel, data->m_posFrontRel);





        m_timer.Enter();

        while (posBack != posFront || immedData) {
            uint16_t sliceCount = 0;
            uint32_t iterationBytesSent = 0;

            m_timer2.Enter();

            // slice area of send buffer into slices fitting receive buffers
            while (sliceCount < queueSize && posBack != posFront || immedData) {
                uint32_t posEnd = posFront;

                // ring buffer wrap around detected: first, send everything
                // up to the end of the buffer (size)
                if (posBack > posFront) {
                    // go to end of buffer, first
                    posEnd = sendBuffer->GetSize();
                }

                // end of buffer reached, wrap around to beginning of buffer
                // and continue
                if (posBack == posEnd) {
                    posBack = 0;
                    posEnd = posFront;
                }


                // fits a full receive buffer
                if (posBack + m_recvBufferSize <= posEnd) {
                    m_sgeLists[sliceCount].addr = (uintptr_t) sendBuffer->GetAddress() + posBack;
                    m_sgeLists[sliceCount].length = m_recvBufferSize;
                    m_sgeLists[sliceCount].lkey = sendBuffer->GetLKey();

                    m_sendWrs[sliceCount].wr_id = 0;
                    m_sendWrs[sliceCount].sg_list = &m_sgeLists[sliceCount];
                    m_sendWrs[sliceCount].num_sge = 1;
                    m_sendWrs[sliceCount].opcode = IBV_WR_SEND_WITH_IMM;
                    m_sendWrs[sliceCount].send_flags = 0;
                    // list is connected further down
                    m_sendWrs[sliceCount].next = nullptr;

                    m_sendWrs[sliceCount].imm_data = (((uint32_t) immedData) << 16) | connection->GetSourceNodeId();

                    posBack += m_recvBufferSize;
                    iterationBytesSent += m_recvBufferSize;
                } else {
                    // smaller than a receive buffer
                    uint32_t size = posEnd - posBack;

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

                    m_sgeLists[sliceCount].addr = (uintptr_t) sendBuffer->GetAddress() + posBack;
                    m_sgeLists[sliceCount].length = size;
                    m_sgeLists[sliceCount].lkey = sendBuffer->GetLKey();

                    m_sendWrs[sliceCount].wr_id = 0;
                    m_sendWrs[sliceCount].sg_list = &m_sgeLists[sliceCount];
                    m_sendWrs[sliceCount].num_sge = 1;
                    m_sendWrs[sliceCount].opcode = IBV_WR_SEND_WITH_IMM;
                    m_sendWrs[sliceCount].send_flags = 0;
                    // list is connected further down
                    m_sendWrs[sliceCount].next = nullptr;

                    m_sendWrs[sliceCount].imm_data = (((uint32_t) immedData) << 16) | connection->GetSourceNodeId();

                    if (!zeroLength) {
                        posBack += size;
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

            // connect all work requests
            for (uint16_t i = 0; i < sliceCount - 1; i++) {
                m_sendWrs[i].next = &m_sendWrs[i + 1];
            }

            // make only the last one yield a completion
            m_sendWrs[sliceCount - 1].send_flags = IBV_SEND_SIGNALED;

            ibv_send_wr* firstBadWr;

            int ret = ibv_post_send(connection->GetQp(0)->GetIbQp(), &m_sendWrs[0], &firstBadWr);
            if (ret != 0) {
                switch (ret) {
                    case ENOMEM:
                        throw core::IbQueueFullException("Send queue full");
                    default:
                        throw core::IbException(
                            "Posting work request to send to queue failed (" +
                            std::string(strerror(ret)));
                }
            }

            m_timer2.Exit();

            m_timer3.Enter();

            while (sliceCount > 0) {
                // TODO 20: hardcoded queue size, needs to be passed to constructor already
                int ret = ibv_poll_cq(connection->GetQp(0)->GetSendQueue()->GetCompQueue()->GetCQ(), 20, m_workComps);
                if (ret < 0) {
                    throw core::IbException("Polling completion queue failed: " +
                                      std::to_string(ret));
                }

                if (ret == 0) {
                    continue;
                }

                for (int i = 0; i < ret; i++) {
                    if (m_workComps[i].status != IBV_WC_SUCCESS) {
                        if (m_workComps[i].status) {
                            switch (m_workComps[i].status) {
                                // a previous work request failed and put the queue into error
                                // state
                                //case IBV_WC_WR_FLUSH_ERR:
                                //    throw IbException("Work completion of recv queue failed, "
                                //        "flush err");

                                case IBV_WC_RETRY_EXC_ERR:
                                    if (m_firstWc) {
                                        throw core::IbException(
                                            "First work completion of queue "
                                                "failed, it's very likely your connection "
                                                "attributes are wrong or the remote site isn't in "
                                                "a state to respond");
                                    } else {
                                        throw core::IbDisconnectedException();
                                    }

                                default:
                                    throw core::IbException(
                                        "Found failed work completion, status " +
                                        std::to_string(m_workComps[i].status));
                            }
                        }
                    }

                    m_firstWc = false;
                }

                m_pollCompCounter += ret;
                sliceCount -= ret;

                break;
            }

            m_timer3.Exit();

            m_sentBytes += iterationBytesSent;
            totalBytesSent += iterationBytesSent;
        }

        m_timer.Exit();

        m_connectionManager->ReturnConnection(connection);

        m_prevDataWritten = totalBytesSent;
        m_sendCounter++;
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

void SendThread::_AfterRunLoop(void)
{
    IBNET_LOG_INFO(
        "Send Thread statistics:\n"
        "Total sent: {} GB\n"
        "In total chunks: {}\n"
        "Flow control confirms: {}\n"
        "Average chunk send size: {} MB\n"
        "Time for sending one chunk: {} ms\n"
        "Throughput: {} MB/sec\n"
        "Slice and send data avarage time: {} ms\n"
        "Poll completion total time: {} ms, count {}\n"
        "Time per completion: {} ms\n",
            ((double) m_sentBytes) / 1024 / 1024 / 1024, m_sendCounter,
            m_sentFlowControlConfirms,
            ((double) m_sentBytes) / m_sendCounter / 1024 / 1024,
            m_timer.GetAvarageTime() * 1000,
            ((double) m_sentBytes) / 1024 / 1024 / m_timer.GetTotalTime(),
            m_timer2.GetAvarageTime() * 1000,
            m_timer3.GetTotalTime() * 1000, m_pollCompCounter,
            m_timer3.GetTotalTime() * 1000 / m_pollCompCounter);
}

}
}