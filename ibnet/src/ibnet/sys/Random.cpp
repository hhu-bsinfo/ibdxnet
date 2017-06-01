#include "Random.h"

#include <stdlib.h>
#include <time.h>

namespace ibnet {
namespace sys {

bool Random::ms_randInit = false;

uint16_t Random::Generate16(void)
{
    return (uint16_t) Generate32();
}

uint32_t Random::Generate32(void)
{
    if (!ms_randInit) {
        srand48(time(NULL));
        ms_randInit = true;
    }

    return lrand48();
}

}
}