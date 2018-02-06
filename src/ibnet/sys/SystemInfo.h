//
// Created by on 2/2/18.
//

#ifndef IBNET_SYS_SYSTEMINFO_H
#define IBNET_SYS_SYSTEMINFO_H

#include <string>

namespace ibnet {
namespace sys {

class SystemInfo
{
public:
    static void LogHardwareReport();

    static void LogOSReport();

    static void LogApplicationReport();

private:
    SystemInfo() = default;

    ~SystemInfo() = default;

    static std::string __ExecuteCmd(const std::string& cmd);
};

}
}

#endif //IBNET_SYS_SYSTEMINFO_H
