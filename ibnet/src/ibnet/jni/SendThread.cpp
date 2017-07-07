#include "SendThread.h"

#include "ibnet/core/IbDisconnectedException.h"

namespace ibnet {
namespace jni {

SendThread::SendThread(std::shared_ptr<core::IbProtDom>& protDom,
        uint32_t outBufferSize, uint32_t bufferQueueSize,
        std::shared_ptr<SendHandler>& sendHandler,
        std::shared_ptr<core::IbConnectionManager>& connectionManager) :
    ThreadLoop("SendThread"),
    m_outBufferSize(outBufferSize),
    m_sendHandler(sendHandler),
    m_connectionManager(connectionManager),
    m_prevNodeIdWritten(core::IbNodeId::INVALID),
    m_prevDataWritten(0),
    m_prevFlowControlWritten(0),
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
    m_timers[1].Enter();

    SendHandler::NextWorkParameters* data = m_sendHandler->GetNextDataToSend(
        m_prevNodeIdWritten, m_prevDataWritten, m_prevFlowControlWritten);

    // nothing to process
    if (data == nullptr) {
        m_timers[1].Exit();
        std::this_thread::yield();
        return;
    }

    m_timers[1].Exit();

    // update and reset previous state
    m_prevNodeIdWritten = data->m_nodeId;
    m_prevFlowControlWritten = 0;
    m_prevDataWritten = 0;

    m_timers[2].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(data->m_nodeId);

    m_timers[2].Exit();

    // connection closed in the meantime
    if (!connection) {
        // sent back to java space on the next GetNext call
        m_prevDataWritten = 0;
        m_prevFlowControlWritten = 0;
        return;
    }

    try {
        m_prevFlowControlWritten = __ProcessFlowControl(connection, data);
        m_prevDataWritten = __ProcessBuffers(connection, data);

        m_connectionManager->ReturnConnection(connection);
    } catch (core::IbQueueClosedException& e) {
        m_connectionManager->ReturnConnection(connection);
        // ignore
    } catch (core::IbDisconnectedException& e) {
        m_connectionManager->ReturnConnection(connection);
        m_connectionManager->CloseConnection(connection->GetRemoteNodeId(), true);
    }
}

void SendThread::_AfterRunLoop(void)
{
    m_timers[0].Exit();
    PrintStatistics();
}

uint32_t SendThread::__ProcessFlowControl(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data)
{
    const uint32_t numBytesToSend = sizeof(data->m_flowControlData);

    if (data->m_flowControlData == 0) {
        return 0;
    }

    m_timers[3].Enter();

    if (!connection->GetQp(1)->GetSendQueue()->Reserve()) {
        m_timers[3].Exit();
        return 0;
    }

    memcpy(m_flowControlBuffer->GetAddress(), &data->m_flowControlData,
        numBytesToSend);

    m_timers[3].Exit();

    m_timers[4].Enter();

    connection->GetQp(1)->GetSendQueue()->Send(m_flowControlBuffer,
        numBytesToSend);
    m_timers[4].Exit();

    m_timers[5].Enter();

    try {
        connection->GetQp(1)->GetSendQueue()->PollCompletion(true);
    } catch (...) {
        m_timers[5].Exit();
        throw;
    }

    m_timers[5].Exit();

    m_sentFlowControlBytes += numBytesToSend;

    return data->m_flowControlData;
}

uint32_t SendThread::__ProcessBuffers(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data)
{
    const uint16_t queueSize = connection->GetQp(0)->GetSendQueue()->GetQueueSize();
    uint16_t wqsToSend;

    // happens if a node disconnected. the write interest can't be removed
    // from the queue on disconnect
    if (data->m_len == 0) {
        return 0;
    }

    wqsToSend = (uint16_t) (data->m_len / m_outBufferSize);
    // at least one buffer if > 0
    wqsToSend++;

    // limit to max queue size
    if (wqsToSend > queueSize) {
        wqsToSend = queueSize;
    }

    uint32_t wqsSent = 0;
    uint32_t totalBytesSent = 0;

    // batch send as many elements possible by utilizing the queue size
    for (uint32_t i = 0; i < wqsToSend; i++) {
        if (!connection->GetQp(0)->GetSendQueue()->Reserve()) {
            break;
        }

        m_timers[6].Enter();

        uint32_t sliceSize = m_outBufferSize;
        if (data->m_len - totalBytesSent < sliceSize) {
            sliceSize = data->m_len - totalBytesSent;
        }

        memcpy(m_buffers[i]->GetAddress(),
            (void*) (data->m_ptrBuffer + totalBytesSent), sliceSize);

        m_timers[6].Exit();

        m_timers[7].Enter();

        connection->GetQp(0)->GetSendQueue()->Send(m_buffers[i], sliceSize);

        m_timers[7].Exit();

        totalBytesSent += sliceSize;
        wqsSent++;
    }

    // poll for send completions
    for (uint32_t i = 0; i < wqsSent; i++) {
        m_timers[8].Enter();

        try {
            connection->GetQp(0)->GetSendQueue()->PollCompletion(true);
        } catch (...) {
            m_timers[8].Exit();
            throw;
        }

        m_timers[8].Exit();
    }

    m_sentBytes += totalBytesSent;

    return totalBytesSent;
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