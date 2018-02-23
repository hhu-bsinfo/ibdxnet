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

//
// Created by on 2/2/18.
//
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
