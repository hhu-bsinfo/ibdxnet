/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
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

#include <signal.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "ibnet/sys/Logger.h"
#include "ibnet/sys/Random.h"

#include "ibnet/core/IbNodeConfArgListReader.h"
#include "ibnet/core/IbDiscoveryManager.h"

static bool g_loop = true;

static void SignalHandler(int signal)
{
    if (signal == SIGINT) {
        g_loop = false;
    }
}

int main(int argc, char** argv)
{
    ibnet::sys::Logger::Setup();

    signal(SIGINT, SignalHandler);

    ibnet::core::IbNodeConfArgListReader nodeConfArgListReader(
            (uint32_t) (argc - 1), &argv[1]);
    ibnet::core::IbNodeConf nodeConf = nodeConfArgListReader.Read();

    uint16_t nodeId = ibnet::sys::Random::Generate16();
    ibnet::core::IbDiscoveryManager discMan(nodeId, nodeConf, 5730);

    std::cout << "Own node id: 0x" <<  std::hex << nodeId << std::endl;
    std::cout << "Running loop..." << std::endl;

    while (g_loop) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}