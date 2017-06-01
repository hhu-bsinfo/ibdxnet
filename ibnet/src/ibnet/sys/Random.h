#ifndef IBNET_SYS_RANDOM_H
#define IBNET_SYS_RANDOM_H

#include <cstdint>

namespace ibnet {
namespace sys {

/**
 * Utility class for generating random numbers
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Random
{
public:
    /**
     * Generate a 16-bit random number
     */
    static uint16_t Generate16(void);

    /**
     * Generate a 32-bit random number
     */
    static uint32_t Generate32(void);

private:
    Random(void) {};
    ~Random(void) {};

    static bool ms_randInit;
};

}
}

#endif // IBNET_SYS_RANDOM_H
