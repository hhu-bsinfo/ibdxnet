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

#ifndef IBNET_VERSION_H
#define IBNET_VERSION_H

/**
 * Version, Gitrev and build date fields set by the cmake scripts
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 02.02.2018
 */

#define XSTRINGIFY(a) STRINGIFY(a)
#define STRINGIFY(a) #a

namespace ibnet {

#ifdef IBNET_VERSION
static const char* VERSION = XSTRINGIFY(IBNET_VERSION);
#else
static const char* VERSION = "0.0";
#endif

#ifdef IBNET_GIT_REV
static const char* GIT_REV = XSTRINGIFY(IBNET_GIT_REV);
#else
static const char* GIT_REV = "not available";
#endif

#ifdef IBNET_BUILD_DATE
static const char* BUILD_DATE = XSTRINGIFY(IBNET_BUILD_DATE);
#else
static const char* BUILD_DATE = "0000-00-00 00:00:00";
#endif

}

#undef STRINGIFY
#undef XSTRINGIFY

#endif //IBNET_VERSION_H
