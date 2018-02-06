//
// Created by on 2/2/18.
//

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