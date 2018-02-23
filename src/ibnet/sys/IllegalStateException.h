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

#ifndef IBNET_SYS_ILLEGALSTATEEXCEPTION_H
#define IBNET_SYS_ILLEGALSTATEEXCEPTION_H

#include "Exception.h"

namespace ibnet {
namespace sys {

/**
 * Exception when encountering an illegal state (e.g. illegal variable value)
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class IllegalStateException : public Exception
{
public:
    /**
     * Constructor
     */
    IllegalStateException() :
        Exception("")
    {
    }

    /**
     * Constructor
     *
     * @param format Printf style format message
     * @param args Parameters for format string
     */
    template <typename... Args>
    explicit IllegalStateException(const std::string& format, Args... args) :
        Exception(format, args...)
    {
    }

    /**
     * Destructor
     */
    ~IllegalStateException() = default;
};

}
}

#endif //IBNET_SYS_ILLEGALSTATEEXCEPTION_H
