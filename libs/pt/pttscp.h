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

#ifndef PTTSCP_H
#define PTTSCP_H

#include <stdint.h>

/**
 * This implementation is based on the following source:
 * https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf
 */

/**
 * Check if the rdtscp instruction is supported by the current CPU
 */
static inline bool pttscp_support()
{
    uint32_t in = 1;
    uint32_t flags;

    asm volatile("cpuid" : "=d" (flags) : "a" (in));
    return flags & (1 << 27);
}

/**
 * Start time measurement
 * 
 * @return Current "time" in clock cycles
 */
static inline uint64_t pttscp_start()
{
    uint32_t lo;
    uint32_t hi;

    asm volatile ("CPUID\n\t" 
        "RDTSC\n\t" 
        "mov %%edx, %0\n\t" 
        "mov %%eax, %1\n\t": "=r" (hi), "=r" (lo):: 
        "%rax", "%rbx", "%rcx", "%rdx");

    return (uint64_t) hi << 32 | lo;
}

/**
 * End time measurement
 * 
 * @return Current "time" in clock cycles
 */
static inline uint64_t pttscp_end()
{
    uint32_t lo;
    uint32_t hi;

    asm volatile("RDTSCP\n\t" 
        "mov %%edx, %0\n\t" 
        "mov %%eax, %1\n\t" 
        "CPUID\n\t": "=r" (hi), "=r" (lo):: "%rax", "%rbx", "%rcx", "%rdx");

    return (uint64_t) hi << 32 | lo;
}

/**
 * Measure the minimal overhead when using rdtscp for time measurements. This
 * value must be considered when measuring times using rdtscp
 * 
 * @return Overead when using rdtscp to measure time in cycles
 */
static uint64_t pttscp_overhead(uint32_t samples)
{
    uint32_t cycles_low;
    uint32_t cycles_high;
    uint32_t cycles_low1;
    uint32_t cycles_high1;

    uint64_t start;
    uint64_t end;
    uint64_t min = 0;
    
    /* Warm up the instruction cache to avoid spurious measurements due to 
       cache effects */

    asm volatile ("CPUID\n\t" 
        "RDTSC\n\t" 
        "mov %%edx, %0\n\t" 
        "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%rax", "%rbx", "%rcx", "%rdx"); 

    asm volatile("RDTSCP\n\t"
         "mov %%edx, %0\n\t" 
         "mov %%eax, %1\n\t" 
         "CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: 
         "%rax", "%rbx", "%rcx", "%rdx"); 
    
    asm volatile ("CPUID\n\t" 
          "RDTSC\n\t" 
          "mov %%edx, %0\n\t" 
          "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
          "%rax", "%rbx", "%rcx", "%rdx"); 
    
    asm volatile("RDTSCP\n\t" 
         "mov %%edx, %0\n\t" 
         "mov %%eax, %1\n\t" 
         "CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: 
         "%rax", "%rbx", "%rcx", "%rdx");

    /* Execute overhead measurements. The minimum is the guaranteed overhead. */

    for (uint32_t i = 0; i < samples; i++) {
        start = pttscp_start();
        end = pttscp_end();

        if (end < start) {
            return (uint64_t) -1;
        }

        if (min == 0 || end - start < min) {
            min = end - start;
        }
    }

    return min;
}

#endif /* PTTSCP_H */
