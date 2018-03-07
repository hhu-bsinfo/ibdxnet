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

#ifndef IBNET_CONFIG_H
#define IBNET_CONFIG_H

/**
 * Edit this file (or add the flags to your compiler options) to configure
 * parts of ibdxnet for better performance or enabling/disable features
 */

/**
 * Removes all trace logger calls entirely before compiling. This greatly
 * enhances performance as well as avoids flooding the log with useless trace
 * messages on normal operation. This option might be useful if you are
 * tracing bugs, only.
 */
#define IBNET_LOG_TRACE_STRIP

/**
 * Compiler flag to disable statistics
 *
 * When executing benchmarks, you should disable any statistics because
 * there are a lot of them used for debugging and to analyze the control flow
 * to detect possible bottlenecks or unoptimized paths.
 */
// #define IBNET_DISABLE_STATISTICS

#endif //IBNET_CONFIG_H
