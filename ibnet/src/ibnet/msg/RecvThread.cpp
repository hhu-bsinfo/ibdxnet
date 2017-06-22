#include <ibnet/core/IbDisconnectedException.h>
#include "RecvThread.h"

#include "MsgException.h"

namespace ibnet {
namespace msg {

RecvThread::RecvThread(
        bool primaryRecvThread,
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<core::IbCompQueue>& sharedFlowControlRecvCQ,
        std::shared_ptr<BufferPool>& recvBufferPool,
        std::shared_ptr<BufferPool>& recvFlowControlBufferPool,
        std::shared_ptr<MessageHandler>& msgHandler) :
    ThreadLoop("RecvThread"),
    m_primaryRecvThread(primaryRecvThread),
    m_connectionManager(connectionManager),
    m_sharedRecvCQ(sharedRecvCQ),
    m_sharedFlowControlRecvCQ(sharedFlowControlRecvCQ),
    m_recvBufferPool(recvBufferPool),
    m_recvFlowControlBufferPool(recvFlowControlBufferPool),
    m_messageHandler(msgHandler),
    m_sharedQueueInitialFill(false),
    m_recvBytes(0),
    m_recvFlowControlBytes(0)
{
    m_timers.push_back(sys::ProfileTimer("Total"));
    m_timers.push_back(sys::ProfileTimer("FCPoll"));
    m_timers.push_back(sys::ProfileTimer("FCGetNodeIdForQp"));
    m_timers.push_back(sys::ProfileTimer("FCHandle"));
    m_timers.push_back(sys::ProfileTimer("FCPostWRQ"));
    m_timers.push_back(sys::ProfileTimer("BufferPoll"));
    m_timers.push_back(sys::ProfileTimer("BufferGetNodeIdForQp"));
    m_timers.push_back(sys::ProfileTimer("BufferHandle"));
    m_timers.push_back(sys::ProfileTimer("BufferPostWRQ"));
}

RecvThread::~RecvThread(void)
{

}

void RecvThread::NodeConnected(core::IbConnection& connection)
{
    // on the first connection, fill the shared recv queue
    // primary recv thread only (on multiple recv threads)
    if (m_primaryRecvThread) {
        bool expected = false;
        if (!m_sharedQueueInitialFill.compare_exchange_strong(expected, true,
                std::memory_order_relaxed)) {
            return;
        }

        // sanity check
        if (!connection.GetQp(0)->GetRecvQueue()->IsRecvQueueShared()) {
            throw MsgException("Can't work with non shared recv queue(s)");
        }

        auto vec = m_recvBufferPool->GetEntries();
        for (auto& it : vec) {
            if (!connection.GetQp(0)->GetRecvQueue()->Reserve()) {
                throw MsgException("Recv queue buffer outstanding overrun");
            }

            connection.GetQp(0)->GetRecvQueue()->Receive(it.m_mem, it.m_id);
        }

        // sanity check
        if (!connection.GetQp(1)->GetRecvQueue()->IsRecvQueueShared()) {
            throw MsgException("Can't work with non shared FC recv queue(s)");
        }

        vec = m_recvFlowControlBufferPool->GetEntries();
        for (auto& it : vec) {
            if (!connection.GetQp(1)->GetRecvQueue()->Reserve()) {
                throw MsgException("Recv queue FC outstanding overrun");
            }

            connection.GetQp(1)->GetRecvQueue()->Receive(it.m_mem, it.m_id);
        }
    }
}

void RecvThread::PrintStatistics(void)
{
    std::cout << "ReceiveThread statistics:" <<
    std::endl <<
    "Throughput: " << m_recvBytes / m_timers[0].GetTotalTime() / 1024.0 / 1024.0 <<
    " MB/sec" << std::endl <<
    "Recv data: " << m_recvBytes / 1024.0 / 1024.0 << " MB" << std::endl <<
    "FC Throughput: " << m_recvFlowControlBytes / m_timers[0].GetTotalTime() / 1024.0 / 1024.0 <<
    " MB/sec" << std::endl <<
    "FC Recv data: " << m_recvFlowControlBytes / 1024.0 / 1024.0 << " MB" << std::endl;

    for (auto& it : m_timers) {
        std::cout << it << std::endl;
    }
}

void RecvThread::_BeforeRunLoop(void)
{
    m_timers[0].Enter();
}

void RecvThread::_RunLoop(void)
{
    // flow control has higher priority, always try this queue first
    if (__ProcessFlowControl()) {
        return;
    }

    __ProcessBuffers();
}

void RecvThread::_AfterRunLoop(void)
{
    m_timers[0].Exit();
    PrintStatistics();
}

bool RecvThread::__ProcessFlowControl(void)
{
    uint32_t qpNum;
    uint64_t workReqId = (uint64_t) -1;
    uint32_t recvLength = 0;

    m_timers[1].Enter();

    try {
        qpNum = m_sharedFlowControlRecvCQ->PollForCompletion(false, &workReqId,
            &recvLength);
    } catch (core::IbException& e) {
        m_timers[1].Exit();
        IBNET_LOG_ERROR("Polling for flow control completion failed: {}",
            e.what());
        return false;
    }

    m_timers[1].Exit();

    // no flow control data available
    if (qpNum == -1) {
        return false;
    }

    m_timers[2].Enter();

    uint16_t sourceNode = m_connectionManager->GetNodeIdForPhysicalQPNum(qpNum);
    const BufferPool::Entry& poolEntry =
        m_recvFlowControlBufferPool->Get((uint32_t) workReqId);
    m_recvFlowControlBytes += recvLength;

    m_timers[2].Exit();

    if (sourceNode == core::IbNodeId::INVALID) {
        IBNET_LOG_ERROR("No node id mapping for qpNum 0x{:x} on FC data recv",
            qpNum);

        // TODO how to add missing receive entry back to connection/queue?

        return false;
    }

    if (m_messageHandler) {
        m_timers[3].Enter();

        m_messageHandler->HandleFlowControlData(sourceNode,
            *((uint32_t*) poolEntry.m_mem->GetAddress()));

        m_timers[3].Exit();
    }

    m_timers[4].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(sourceNode);

    if (!connection->GetQp(1)->GetRecvQueue()->Reserve()) {
        m_connectionManager->ReturnConnection(connection);
        throw MsgException("Recv queue FC outstanding overrun");
    }

    // keep the recv queue filled, using a shared recv queue here
    connection->GetQp(1)->GetRecvQueue()->Receive(
        poolEntry.m_mem, poolEntry.m_id);

    m_connectionManager->ReturnConnection(connection);

    m_timers[4].Exit();

    return true;
}

bool RecvThread::__ProcessBuffers(void)
{
    uint32_t qpNum;
    uint64_t workReqId = (uint64_t) -1;
    uint32_t recvLength = 0;

    m_timers[5].Enter();

    try {
        qpNum = m_sharedRecvCQ->PollForCompletion(false, &workReqId,
            &recvLength);
    } catch (core::IbException& e) {
        m_timers[5].Exit();
        IBNET_LOG_ERROR("Polling for flow control completion failed: {}",
            e.what());
        return false;
    }

    m_timers[5].Exit();

    // no data available
    if (qpNum == -1) {
        return false;
    }

    m_timers[6].Enter();

    uint16_t sourceNode = m_connectionManager->GetNodeIdForPhysicalQPNum(qpNum);
    const BufferPool::Entry& poolEntry =
        m_recvBufferPool->Get((uint32_t) workReqId);
    m_recvBytes += recvLength;

    m_timers[6].Exit();

    if (sourceNode == core::IbNodeId::INVALID) {
        IBNET_LOG_ERROR("No node id mapping for qpNum 0x{:x} on buffer recv",
            qpNum);

        // TODO how to add missing receive entry back to connection/queue?

        return false;
    }

    if (m_messageHandler) {
        m_timers[7].Enter();

        m_messageHandler->HandleMessage(sourceNode,
            poolEntry.m_mem->GetAddress(), recvLength);

        m_timers[7].Exit();
    }

    m_timers[8].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(sourceNode);

    if (!connection->GetQp(0)->GetRecvQueue()->Reserve()) {
        m_connectionManager->ReturnConnection(connection);
        throw MsgException("Recv queue FC outstanding overrun");
    }

    // keep the recv queue filled, using a shared recv queue here
    connection->GetQp(0)->GetRecvQueue()->Receive(
        poolEntry.m_mem, poolEntry.m_id);

    m_connectionManager->ReturnConnection(connection);

    m_timers[8].Exit();

    return true;
}

}
}