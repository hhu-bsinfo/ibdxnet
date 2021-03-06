/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef PTTSC_H
#define PTTSC_H

#include <stdint.h>

/**
 * This implementation is based on the following source:
 * https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf
 */

/**
 * Check if the rdtsc instruction is supported by the current CPU
 */
static inline bool pttsc_support()
{
    uint32_t in = 1;
    uint32_t flags;

    asm volatile("cpuid" : "=d" (flags) : "a" (in));
    return (bool) (flags & (1 << 4));
}

/**
 * Start time measurement
 *
 * @return Current "time" in clock cycles
 */
static inline uint64_t pttsc_start()
{
    uint32_t lo;
    uint32_t hi;

    asm volatile("cpuid\n\t"
        "rdtsc\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t": "=r" (hi), "=r" (lo)::
        "%rax", "%rbx", "%rcx", "%rdx");

    return (uint64_t) hi << 32 | lo;
}

/**
 * End time measurement (weak version)
 *
 * This version has a lower overhead because it doesn't
 * serialize after the rdtsc instruction which avoids
 * out of order execution of all following instructions
 * together with the rdtsc instruction. This might result
 * in minor inaccuracies but yields better performance.
 *
 * @return Current "time" in clock cycles
 */
static inline uint64_t pttsc_end_weak()
{
    uint32_t lo;
    uint32_t hi;

    asm volatile("cpuid\n\t"
        "rdtsc\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t": "=r" (hi), "=r" (lo)::
        "%rax", "%rbx", "%rcx", "%rdx");

    return (uint64_t) hi << 32 | lo;
}

/**
 * End time measurement (strong version)
 *
 * This version has a higher overhead than the weak one
 * but guarantees higher accuracy for the measured time.
 * It serializes execution after the rdtsc instruction
 * to avoid out of order execution of any following
 * instruction before the rdtsc call completed. However,
 * this comes at a cost of higher overhead.
 *
 * @return Current "time" in clock cycles
 */
static inline uint64_t pttsc_end_strong()
{
    uint32_t lo;
    uint32_t hi;

    asm volatile("cpuid\n\t"
        "rdtsc\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t": "=r" (hi), "=r" (lo)::
        "%rax", "%rbx", "%rcx", "%rdx");

    return (uint64_t) hi << 32 | lo;
}

/**
 * Measure the minimal overhead when using rdtsc for time measurements. This
 * value must be considered when measuring times using rdtsc
 *
 * @return Overead when using rdtsc to measure time in cycles
 */
static uint64_t pttsc_overhead(uint32_t samples)
{
    uint64_t start;
    uint64_t end;
    uint64_t min = 0;

    /* Note: We can't use the alternative (and better) variant which uses the cr0
       register to serialize instructions because the cr0 register is protected
       and can't be accessed in user mode */

    /* Warm up the instruction cache to avoid spurious measurements due to cache effects */

    pttsc_start();
    pttsc_end_strong();
    pttsc_start();
    pttsc_end_strong();

    /* Execute overhead measurements. The minimum is the guaranteed overhead. */

    for (uint32_t i = 0; i < samples; i++) {
        start = pttsc_start();
        end = pttsc_end_strong();

        if (end < start) {
            return (uint64_t) -1;
        }

        if (min == 0 || end - start < min) {
            min = end - start;
        }
    }

    return min;
}

#endif /* PTTSC_H */
