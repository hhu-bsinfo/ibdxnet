# Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
# Institute of Computer Science, Department Operating Systems
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>

include(ExternalProject)

ExternalProject_Add(IbPerfLib_git
        GIT_REPOSITORY https://github.com/hhu-bsinfo/ibperf-lib.git
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/IbPerfLib
        INSTALL_COMMAND "")

ExternalProject_Get_Property(IbPerfLib_git binary_dir)
ExternalProject_Get_Property(IbPerfLib_git source_dir)

add_library(IbPerfLib STATIC IMPORTED)
set_property(TARGET IbPerfLib PROPERTY IMPORTED_LOCATION ${binary_dir}/ar/libIbPerfLib.a)

add_dependencies(IbPerfLib IbPerfLib_git)

include_directories(${source_dir}/src)