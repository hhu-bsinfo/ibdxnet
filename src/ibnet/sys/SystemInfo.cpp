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

#include "SystemInfo.h"

#include "ibnet/Version.h"

#include "Logger.hpp"
#include "Network.h"
#include "SystemException.h"

namespace ibnet {
namespace sys {

void SystemInfo::LogHardwareReport()
{
    IBNET_LOG_INFO("Hardware report of current instance\n"
        "=============================== CPU ===============================\n"
        "%s"
        "============================= Memory =============================\n"
        "%s"
        "===================================================================",
        __ExecuteCmd("cat /proc/cpuinfo"),
        __ExecuteCmd("cat /proc/meminfo"));
}

void SystemInfo::LogOSReport()
{
    IBNET_LOG_INFO("OS report of current instance\n"
        "OS: %s"
        "Host: %s",
        __ExecuteCmd("uname -a"),
        Network::GetHostname());
}

void SystemInfo::LogApplicationReport()
{
    IBNET_LOG_INFO("Application report of current process\n"
        "Version: %s\n"
        "Build date: %s\n"
        "Git Rev: %s",
        ibnet::VERSION, ibnet::BUILD_DATE, ibnet::GIT_REV);
}

std::string SystemInfo::__ExecuteCmd(const std::string& cmd)
{
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe == nullptr) {
        throw sys::SystemException("Can't open pipe to %s: %s",
            cmd.c_str(), strerror(errno));
    }

    char buffer[1024 * 4];
    std::string str;
    size_t read;

    while (true) {
        read = fread(buffer, 1, sizeof(buffer), pipe);

        if (read > 0) {
            str += std::string(buffer, read);
        } else {
            break;
        }
    }

    pclose(pipe);

    return str;
}

}
}