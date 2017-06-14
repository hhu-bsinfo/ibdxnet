#include "SendThread.h"

#include "ibnet/core/IbDisconnectedException.h"

namespace ibnet {
namespace msg {

SendThread::SendThread(std::shared_ptr<core::IbProtDom>& protDom,
        uint32_t outBufferSize, uint32_t bufferQueueSize,
        std::shared_ptr<SendQueues>& bufferSendQueues,
        std::shared_ptr<core::IbConnectionManager>& connectionManager) :
    ThreadLoop("SendThread"),
    m_bufferSendQueues(bufferSendQueues),
    m_connectionManager(connectionManager),
    m_sentBytes(0),
    m_sentFlowControlBytes(0)
{
    m_flowControlBuffer = __AllocAndRegisterMem(protDom, sizeof(uint32_t));

    for (uint32_t i = 0; i < bufferQueueSize; i++) {
        m_buffers.push_back(__AllocAndRegisterMem(protDom, outBufferSize));
    }

    m_timers.push_back(sys::ProfileTimer("Total"));
    m_timers.push_back(sys::ProfileTimer("NextJob"));
    m_timers.push_back(sys::ProfileTimer("GetConnection"));
    m_timers.push_back(sys::ProfileTimer("FCCopy"));
    m_timers.push_back(sys::ProfileTimer("FCSend"));
    m_timers.push_back(sys::ProfileTimer("FCPoll"));
    m_timers.push_back(sys::ProfileTimer("BufferCopy"));
    m_timers.push_back(sys::ProfileTimer("BufferSend"));
    m_timers.push_back(sys::ProfileTimer("BufferPoll"));
}

SendThread::~SendThread(void)
{

}

void SendThread::PrintStatistics(void)
{
    std::cout << "SendThread statistics:" <<
    std::endl <<
    "Throughput: " << m_sentBytes / m_timers[0].GetTotalTime() / 1024.0 / 1024.0 <<
    " MB/sec" << std::endl <<
    "Sent data: " << m_sentBytes / 1024.0 / 1024.0 << " MB" << std::endl <<
    "FC Throughput: " << m_sentFlowControlBytes / m_timers[0].GetTotalTime() / 1024.0 / 1024.0 <<
    " MB/sec" << std::endl <<
    "FC Sent data: " << m_sentFlowControlBytes / 1024.0 / 1024.0 << " MB" << std::endl;

    for (auto& it : m_timers) {
        std::cout << it << std::endl;
    }
}

void SendThread::_BeforeRunLoop(void)
{
    m_timers[0].Enter();
}

void SendThread::_RunLoop(void)
{
    uint16_t targetNodeId;
    uint16_t connectionId;
    std::shared_ptr<std::atomic<uint32_t>> flowControlData;
    std::shared_ptr<ibnet::sys::Queue<std::shared_ptr<SendData>>> queue;

    m_timers[1].Enter();

    if (!m_bufferSendQueues->Next(targetNodeId, connectionId, flowControlData,
            queue)) {
        m_timers[1].Exit();
        std::this_thread::yield();
        return;
    }

    m_timers[1].Exit();

    m_timers[2].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(targetNodeId);

    m_timers[2].Exit();

    uint32_t consumedInterests = 0;
    try {
        consumedInterests += __ProcessFlowControl(targetNodeId, connection, flowControlData);
        consumedInterests += __ProcessBuffers(targetNodeId, connection, queue);
    } catch (...) {
        m_bufferSendQueues->Finished(connection->GetConnectionId(), consumedInterests);
        throw;
    }

    m_bufferSendQueues->Finished(connection->GetConnectionId(), consumedInterests);
}

void SendThread::_AfterRunLoop(void)
{
    m_timers[0].Exit();
    PrintStatistics();
}

uint32_t SendThread::__ProcessFlowControl(
        uint16_t nodeId, std::shared_ptr<core::IbConnection>& connection,
        std::shared_ptr<std::atomic<uint32_t>>& flowControlData)
{
    uint32_t numBytesToSend = sizeof(uint32_t);
    uint32_t data;

    m_timers[3].Enter();

    if (!connection->GetQp(1)->GetSendQueue()->Reserve()) {
        return 0;
    }

    data = flowControlData->load(std::memory_order_relaxed);

    while (data != 0) {
        if (flowControlData->compare_exchange_weak(data, 0, std::memory_order_relaxed)) {
            break;
        }
    }

    if (data == 0) {
        connection->GetQp(1)->GetSendQueue()->RevokeReservation();
        return 0;
    }

    memcpy(m_flowControlBuffer->GetAddress(), &data, numBytesToSend);

    m_timers[3].Exit();

    m_timers[4].Enter();

    connection->GetQp(1)->GetSendQueue()->Send(m_flowControlBuffer,
        numBytesToSend);
    m_timers[4].Exit();

    m_timers[5].Enter();

    try {
        connection->GetQp(1)->GetSendQueue()->PollCompletion(true);
    } catch (core::IbDisconnectedException& e) {
        m_timers[5].Exit();
        uint16_t connectionId = connection->GetConnectionId();
        connection.reset();
        m_connectionManager->CloseConnection(nodeId, true);

        m_bufferSendQueues->NodeDisconnected(connectionId);
        return 0;
    }

    m_timers[5].Exit();

    m_sentFlowControlBytes += numBytesToSend;

    return 1;
}

uint32_t SendThread::__ProcessBuffers(uint16_t nodeId,
        std::shared_ptr<core::IbConnection>& connection,
        std::shared_ptr<ibnet::sys::Queue<std::shared_ptr<SendData>>>& queue)
{
    uint16_t queueSize = connection->GetQp(0)->GetSendQueue()->GetQueueSize();
    uint32_t elemsToSend = queue->GetElementCount();

    // happens if a node disconnected. the write interest can't be removed
    // from the queue on disconnect
    if (elemsToSend == 0) {
        return 0;
    }

    if (elemsToSend > queueSize) {
        elemsToSend = queueSize;
    }

    uint32_t elemsSent = 0;
    uint32_t numBytesToSend = 0;
    uint32_t totalBytesSent = 0;

    // batch send as many elements possible by utilizing the queue size
    for (uint32_t i = 0; i < elemsToSend; i++) {
        if (!connection->GetQp(0)->GetSendQueue()->Reserve()) {
            break;
        }

        std::shared_ptr<SendData> data;
        if (!queue->PopFront(data)) {
            connection->GetQp(0)->GetSendQueue()->RevokeReservation();
            break;
        }

        m_timers[6].Enter();

        numBytesToSend = data->m_size;
        memcpy(m_buffers[i]->GetAddress(), data->m_buffer, data->m_size);
        data.reset();

        m_timers[6].Exit();

        m_timers[7].Enter();

        connection->GetQp(0)->GetSendQueue()->Send(m_buffers[i], numBytesToSend);

        m_timers[7].Exit();

        elemsSent++;
        totalBytesSent += numBytesToSend;
    }

    // poll for send completions
    for (uint32_t i = 0; i < elemsSent; i++) {
        m_timers[8].Enter();

        try {
            connection->GetQp(0)->GetSendQueue()->PollCompletion(true);
        } catch (core::IbDisconnectedException& e) {
            m_timers[8].Exit();
            uint16_t connectionId = connection->GetConnectionId();
            connection.reset();
            m_connectionManager->CloseConnection(nodeId, true);

            m_bufferSendQueues->NodeDisconnected(connectionId);

            return elemsSent;
        }

        m_timers[8].Exit();
    }

    m_sentBytes += totalBytesSent;

    return elemsSent;
}

std::shared_ptr<core::IbMemReg> SendThread::__AllocAndRegisterMem(
        std::shared_ptr<core::IbProtDom>& protDom, uint32_t size)
{
    void* buffer = malloc(size);
    memset(buffer, 0, size);

    return protDom->Register(buffer, size, true);
}

}
}