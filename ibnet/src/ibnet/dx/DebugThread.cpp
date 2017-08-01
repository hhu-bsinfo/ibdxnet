#include "DebugThread.h"

namespace ibnet {
namespace dx {

DebugThread::DebugThread(
        std::shared_ptr<RecvThread> recvThread,
        std::shared_ptr<SendThread> sendThread) :
    m_recvThread(recvThread),
    m_sendThread(sendThread)
{

}

DebugThread::~DebugThread(void)
{

}

void DebugThread::_RunLoop(void)
{
    m_sendThread->PrintStatistics();
    m_recvThread->PrintStatistics();

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

}
}