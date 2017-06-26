#include "DebugThread.h"

namespace ibnet {
namespace jni {

DebugThread::DebugThread(std::shared_ptr<ibnet::msg::IbMessageSystem> system) :
    m_system(system)
{

}

DebugThread::~DebugThread(void)
{

}

void DebugThread::_RunLoop(void)
{
    m_system->PrintStatus();

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

}
}