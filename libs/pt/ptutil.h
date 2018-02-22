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

#ifndef PTUTIL_H
#define PTUTIL_H

#include <stdlib.h>
#include <sys/time.h>

#include "pttsc.h"
#include "pttscp.h"

/**
 * Convert cycles to seconds
 * 
 * @param cycles The number of cycles to convert
 * @param cycles_per_sec Number of cycles per second
 * @return Time in seconds
 */
static inline uint64_t ptutil_cycles_to_sec(uint64_t cycles, 
        double cycles_per_sec)
{
    return cycles / cycles_per_sec;
}

/**
 * Convert cycles to milliseconds
 * 
 * @param cycles The number of cycles to convert
 * @param cycles_per_sec Number of cycles per second
 * @return Time in milliseconds
 */
static inline uint64_t ptutil_cycles_to_ms(uint64_t cycles, 
        double cycles_per_sec)
{
    return cycles / (cycles_per_sec / 1000.0);
}

/**
 * Convert cycles to microseconds
 * 
 * @param cycles The number of cycles to convert
 * @param cycles_per_sec Number of cycles per second
 * @return Time in microseconds
 */
static inline uint64_t ptutil_cycles_to_us(uint64_t cycles, 
        double cycles_per_sec)
{
    return cycles / (cycles_per_sec / 1000.0 / 1000.0);
}

/**
 * Convert cycles to nanoseconds
 * 
 * @param cycles The number of cycles to convert
 * @param cycles_per_sec Number of cycles per second
 * @return Time in nanoseconds
 */
static inline uint64_t ptutil_cycles_to_ns(uint64_t cycles, 
        double cycles_per_sec)
{
    return cycles / (cycles_per_sec / 1000.0 / 1000.0 / 1000.0);
}

/**
 * Source: https://github.com/PlatformLab/RAMCloud/blob/master/src/Cycles.cc
 * 
 * Calculate the number of cycles per second which is used to convert cycle
 * counts yielded by rdtsc or rdtscp to secs, ms, us or ns.
 * 
 * @param pt_start_func rdtsc or rdtscp start function
 * @param pt_end_func rdtsc or rdtscp end function
 * @param overhead_cycles The overhead (in cycles) to consider when measuring 
 *        time using rdtsc or rdtscp
 * @return Number of cycles per second
 */
static double ptutil_cycles_per_sec(uint64_t (*pt_start_func)(), 
        uint64_t (*pt_end_func)(), uint64_t overhead_cycles)
{
    /* Compute the frequency of the fine-grained CPU timer: to do this,
       take parallel time readings using both rdtsc and gettimeofday.
       After 10ms have elapsed, take the ratio between these readings. */

    double cycles_per_sec;
    struct timeval start_time;
    struct timeval stop_time;
    uint64_t start_cycles;
    uint64_t stop_cycles;
    uint64_t micros;
    double old_cycles;

    /* There is one tricky aspect, which is that we could get interrupted
       between calling gettimeofday and reading the cycle counter, in which
       case we won't have corresponding readings.  To handle this (unlikely)
       case, compute the overall result repeatedly, and wait until we get
       two successive calculations that are within 0.1% of each other. */

    old_cycles = 0;

    while (1) {
        if (gettimeofday(&start_time, NULL) != 0) {
            return 0.0;
        }

        start_cycles = pt_start_func();

        while (1) {
            if (gettimeofday(&stop_time, NULL) != 0) {
                return 0.0;
            }
                    
            stop_cycles = pt_end_func();
            micros = (stop_time.tv_usec - start_time.tv_usec) + 
                (stop_time.tv_sec - start_time.tv_sec) * 1000000;

            if (micros > 10000) {
                cycles_per_sec = 
                    (double) (stop_cycles - start_cycles - overhead_cycles);
                cycles_per_sec = 1000000.0 * cycles_per_sec/
                        (double) (micros);
                break;
            }
        }

        double delta = cycles_per_sec / 1000.0;

        if ((old_cycles > (cycles_per_sec - delta)) &&
                (old_cycles < (cycles_per_sec + delta))) {
            return cycles_per_sec;
        }

        old_cycles = cycles_per_sec;
    }

    return cycles_per_sec;
}

#endif /* PTUTIL_H */
