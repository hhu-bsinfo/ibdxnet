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

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include "ibnet/sys/Logger.h"
#include "ibnet/core/IbDevice.h"

static bool g_loop = true;

/**
 * Signal handler to abort execution (on SIGINT)
 *
 * @param signal Signal triggered
 */
static void SignalHandler(int signal)
{
    if (signal == SIGINT) {
        g_loop = false;
    }
}

/**
 * Main entry point
 *
 * @param argc Argc
 * @param argv Argc
 * @return Exit code
 */
int main(int argc, char** argv)
{
    ibnet::sys::Logger::Setup();

    signal(SIGINT, SignalHandler);

    ibnet::core::IbDevice device;

    std::cout << "Running loop..." << std::endl;

    while (g_loop) {
        device.UpdateState();
        std::cout << device << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Cleanup..." << std::endl;

    return 0;
}