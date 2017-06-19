#include <iostream>

#include "ibnet/sys/Debug.h"

int main(int argc, char** argv)
{
    std::cout << "Wait for debugger to attach" << std::endl;
    ibnet::sys::Debug::WaitForDebuggerToAttach();

    std::cout << "Software breakpoint" << std::endl;
    ibnet::sys::Debug::DebugBreak();

    std::cout << "Done" << std::endl;

    return 0;
}