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

#ifndef IBNET_SYS_SYSTEMINFO_H
#define IBNET_SYS_SYSTEMINFO_H

#include <string>

namespace ibnet {
namespace sys {

/**
 * Access information about the current system (hardware, os, kernel etc)
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.02.2018
 */
class SystemInfo
{
public:
    /**
     * Log a hardware report containing cpu and memory information using the logger
     */
    static void LogHardwareReport();

    /**
     * Log information about the current os to the logger
     */
    static void LogOSReport();

    /**
     * Log application information (version, gitrev, build date) to the logger
     */
    static void LogApplicationReport();

private:
    /**
     * Constructor
     */
    SystemInfo() = default;

    /**
     * Destructor
     */
    ~SystemInfo() = default;

    /**
     * Execute a shell command
     *
     * @param cmd Command to execute on the shell
     * @return Stdout of command stored as string
     */
    static std::string __ExecuteCmd(const std::string& cmd);
};

}
}

#endif //IBNET_SYS_SYSTEMINFO_H
