#ifndef IBNET_SYS_DEBUG_H
#define IBNET_SYS_DEBUG_H

#include <iostream>

/**
 * Use this to quickly hack in debug prints which can be removed
 * by the pre-processor
 */
#define DBGPRINT(str) std::cout << ">>>>>>>>>> " << str << std::endl

#endif //IBNET_SYS_DEBUG_H
