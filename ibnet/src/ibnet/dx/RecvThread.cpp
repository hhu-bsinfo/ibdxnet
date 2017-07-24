#include "RecvThread.h"

#include "ibnet/core/IbDisconnectedException.h"

#include "DxnetException.h"

namespace ibnet {
namespace dx {

RecvThread::RecvThread(
        bool primaryRecvThread,
        std::shared_ptr<core::IbConnectionManager>& connectionManager,
        std::shared_ptr<core::IbCompQueue>& sharedRecvCQ,
        std::shared_ptr<core::IbCompQueue>& sharedFlowControlRecvCQ,
        std::shared_ptr<RecvBufferPool>& recvBufferPool,
        std::shared_ptr<RecvHandler>& recvHandler) :
    ThreadLoop("RecvThread"),
    m_primaryRecvThread(primaryRecvThread),
    m_connectionManager(connectionManager),
    m_sharedRecvCQ(sharedRecvCQ),
    m_sharedFlowControlRecvCQ(sharedFlowControlRecvCQ),
    m_recvBufferPool(recvBufferPool),
    m_recvHandler(recvHandler),
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

    core::IbMemReg* mem = (core::IbMemReg*) workReqId;
    m_recvFlowControlBytes += recvLength;

    m_timers[2].Exit();

    if (sourceNode == core::IbNodeId::INVALID) {
        IBNET_LOG_ERROR("No node id mapping for qpNum 0x{:x} on FC data recv",
            qpNum);

        // TODO how to add missing receive entry back to connection/queue?

        return false;
    }

    m_timers[3].Enter();

    m_recvHandler->ReceivedFlowControlData(sourceNode,
        *((uint32_t*) mem->GetAddress()));

    m_timers[3].Exit();

    m_timers[4].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(sourceNode);

    // keep the recv queue filled, using a shared recv queue here
    connection->GetQp(1)->GetRecvQueue()->Receive(mem, (uint64_t) mem);

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
    core::IbMemReg* mem = (core::IbMemReg*) workReqId;
    m_recvBytes += recvLength;

    m_timers[6].Exit();

    if (sourceNode == core::IbNodeId::INVALID) {
        IBNET_LOG_ERROR("No node id mapping for qpNum 0x{:x} on buffer recv",
            qpNum);

        // TODO how to add missing receive entry back to connection/queue?

        return false;
    }

    m_timers[7].Enter();

    // pass to jvm space
    // buffer is return to the pool async
    m_recvHandler->ReceivedBuffer(sourceNode, mem, mem->GetAddress(),
        recvLength);

    m_timers[7].Exit();

    m_timers[8].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(sourceNode);

    // keep the recv queue filled, using a shared recv queue here
    // get another buffer from the pool
    core::IbMemReg* buf = m_recvBufferPool->GetBuffer();

    // Use the pointer as the work req id
    connection->GetQp(0)->GetRecvQueue()->Receive(buf,
        (uint64_t) buf);

    m_connectionManager->ReturnConnection(connection);

    m_timers[8].Exit();

    return true;
}

}
}