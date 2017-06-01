#ifndef IBNET_SYS_RANDOM_H
#define IBNET_SYS_RANDOM_H

#include <cstdint>

namespace ibnet {
namespace sys {

class Random
{
public:
    static uint16_t Generate16(void);

    static uint32_t Generate32(void);

private:
    Random(void) {};
    ~Random(void) {};

    static bool ms_randInit;
};

}
}

#endif // IBNET_SYS_RANDOM_H
