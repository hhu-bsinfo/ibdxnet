#include "SendThread.h"

#include "ibnet/core/IbDisconnectedException.h"

namespace ibnet {
namespace dx {

SendThread::SendThread(std::shared_ptr<SendBuffers> buffers,
        std::shared_ptr<SendHandler>& sendHandler,
        std::shared_ptr<core::IbConnectionManager>& connectionManager) :
    ThreadLoop("SendThread"),
    m_buffers(buffers),
    m_sendHandler(sendHandler),
    m_connectionManager(connectionManager),
    m_prevNodeIdWritten(core::IbNodeId::INVALID),
    m_prevDataWritten(0),
    m_sentBytes(0),
    m_sentFlowControlBytes(0)
{

    // TODO rename timers
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
        m_prevNodeIdWritten, m_prevDataWritten);

    m_timers[1].Exit();

    // reset previous state
    m_prevNodeIdWritten = core::IbNodeId::INVALID;
    m_prevDataWritten = 0;

    // nothing to process
    if (data == nullptr) {
        std::this_thread::yield();
        return;
    }

    m_prevNodeIdWritten = data->m_nodeId;

    m_timers[2].Enter();

    std::shared_ptr<core::IbConnection> connection =
        m_connectionManager->GetConnection(data->m_nodeId);

    m_timers[2].Exit();

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
    const uint32_t numBytesToSend = sizeof(uint32_t);

    if (data->m_flowControlData == 0) {
        return 0;
    }

    m_timers[3].Enter();

    core::IbMemReg* mem = m_buffers->GetFlowControlBuffer(
        connection->GetConnectionId());

    memcpy(mem->GetAddress(), &data->m_flowControlData, numBytesToSend);

    m_timers[3].Exit();

    m_timers[4].Enter();

    connection->GetQp(1)->GetSendQueue()->Send(mem, 0, numBytesToSend);

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

uint32_t SendThread::__ProcessBuffer(
        std::shared_ptr<core::IbConnection>& connection,
        SendHandler::NextWorkParameters* data)
{
    uint32_t startOffset = 0;
    uint32_t workRequests = 0;
    uint32_t totalBytesSent = 0;


    m_timers[6].Enter();

    core::IbMemReg* mem = m_buffers->GetBuffer(connection->GetConnectionId());

    m_timers[6].Exit();

    if (data->m_posBackRel < data->m_posFrontRel) {
        // two WQs because ring buffer wrap around
        // send slice up to the buffer's end first

        m_timers[7].Enter();

        connection->GetQp(0)->GetSendQueue()->Send(mem, data->m_posFrontRel,
            mem->GetSize() - data->m_posFrontRel);

        m_timers[7].Exit();

        totalBytesSent += mem->GetSize() - data->m_posFrontRel;
        workRequests++;
        startOffset = 0;
    } else {
        // single WQ
        startOffset = data->m_posFrontRel;
    }

    m_timers[7].Enter();

    connection->GetQp(0)->GetSendQueue()->Send(mem, startOffset,
        data->m_posBackRel - startOffset);

    m_timers[7].Exit();

    totalBytesSent += data->m_posBackRel - startOffset;
    workRequests++;

    // poll for send completions
    for (uint32_t i = 0; i < workRequests; i++) {
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

}
}