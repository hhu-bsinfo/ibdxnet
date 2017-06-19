#ifndef IBNET_SYS_DEBUG_H
#define IBNET_SYS_DEBUG_H

#include <iostream>

/**
 * Use this to quickly hack in debug prints which can be removed
 * by the pre-processor
 */
#define DBGPRINT(str) std::cout << ">>>>>>>>>> " << str << std::endl

namespace ibnet {
namespace sys {

class Debug
{
public:
    static void WaitForDebuggerToAttach(void);

    static void DebugBreak(void);

private:
    Debug(void) {};

    ~Debug(void) {};
};

}
}

#endif //IBNET_SYS_DEBUG_H
