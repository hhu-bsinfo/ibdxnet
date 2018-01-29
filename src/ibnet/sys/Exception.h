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

#ifndef IBNET_SYS_EXCEPTION_H
#define IBNET_SYS_EXCEPTION_H

#include <cstdarg>
#include <cstdio>
#include <exception>
#include <string>

#include <backwards/backward.hpp>
#include <fmt/printf.h>

namespace ibnet {
namespace sys {

/**
 * Exception base class
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Exception : public std::runtime_error
{
public:
   /**
    * Constructor
    *
    * @param format Printf style format message
    * @param args Parameters for format string
    */
    template<typename... Args>
    explicit Exception(const std::string& format, Args... args) :
        std::runtime_error(fmt::sprintf(format, args...))
    {
        m_stackTrace.load_here(32);
    }

    /**
     * Constructor
     *
     * @param msg Exception message (c-string)
     */
    explicit Exception(const char* msg) :
            std::runtime_error(msg)
    {
        m_stackTrace.load_here(32);
    }

    /**
     * Print the stack trace where the exception was created
     */
    void PrintStackTrace() {
        backward::Printer p;
        p.print(m_stackTrace);
    }

private:
    backward::StackTrace m_stackTrace;
};

}
}


#endif //IBNET_SYS_EXCEPTION_H
