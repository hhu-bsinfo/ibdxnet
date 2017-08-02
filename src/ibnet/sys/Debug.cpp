#include "Debug.h"

#include <csignal>

namespace ibnet {
namespace sys {

void Debug::WaitForDebuggerToAttach(void)
{
    std::cout << "Waiting for debugger to attach..." << std::endl;
    raise(SIGSTOP);
    std::cout << "Debugger attached" << std::endl;
}

void Debug::DebugBreak(void)
{
    asm("int $3");
}

}
}