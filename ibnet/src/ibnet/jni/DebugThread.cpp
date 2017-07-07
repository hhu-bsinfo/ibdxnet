#include "DebugThread.h"

namespace ibnet {
namespace jni {

DebugThread::DebugThread(
        const std::vector<std::unique_ptr<ibnet::jni::RecvThread>>& recvThreads,
        const std::vector<std::unique_ptr<ibnet::jni::SendThread>>& sendThreads) :
    m_recvThreads(recvThreads),
    m_sendThreads(sendThreads)
{

}

DebugThread::~DebugThread(void)
{

}

void DebugThread::_RunLoop(void)
{
    std::cout << "Send threads:" << std::endl;
    for (auto& it : m_sendThreads) {
        it->PrintStatistics();
    }

    std::cout << "Recv threads:" << std::endl;
    for (auto& it : m_recvThreads) {
        it->PrintStatistics();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

}
}