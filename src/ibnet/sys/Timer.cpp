#include "Timer.hpp"

namespace ibnet {
namespace sys {

uint64_t Timer::ms_overhead = 0;
double Timer::ms_cyclesPerSec = 0.0;

}
}