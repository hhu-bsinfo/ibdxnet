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

cmake_minimum_required(VERSION 3.5)

# set version
set(IBNET_VERSION "0.1")

# set git revision
execute_process(COMMAND git log -1 --format=%h --date=short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE IBNET_GIT_REV ERROR_QUIET)
string(STRIP "${IBNET_GIT_REV}" IBNET_GIT_REV)
if (IBNET_GIT_REV STREQUAL "")
    set(IBNET_GIT_REV "not available")
endif ()

# set build date
string(TIMESTAMP IBNET_BUILD_DATE "%Y-%m-%d %H:%M:%S")

# apply as proprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIBNET_VERSION='${IBNET_VERSION}' -DIBNET_GIT_REV='${IBNET_GIT_REV}' -DIBNET_BUILD_DATE='${IBNET_BUILD_DATE}'")

message(STATUS "Ibdxnet version ${IBNET_VERSION}, git rev ${IBNET_GIT_REV}, build date ${IBNET_BUILD_DATE}")

# set directory variables
set(IBNET_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(IBNET_SRC_DIR "${IBNET_ROOT_DIR}/src")
set(IBNET_LIBS_DIR "${IBNET_ROOT_DIR}/libs")

# c++ standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# force out of source build
message(STATUS "Force out of source build check...")
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
if (insource)
    message(FATAL_ERROR "Do not build in your source dir. Use a dedicated build folder. Now go cleanup the cache in the CMakeCache.txt and CMakeFiles folder which don't belong there!")
endif ()

# binary/library output dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ar)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

# For development: Uncomment these to compile with sanitizers to debug
# memory errors and leaks
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=null -fsanitize=return -fsanitize=bounds -lasan -lubsan")
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize-recover=all")

# sub-projects
add_subdirectory(fmt)

add_subdirectory(ConnectionManagerTest)
add_subdirectory(IbDeviceTest)
add_subdirectory(IbAddressHandleTest)
add_subdirectory(IbnetCon)
add_subdirectory(IbnetCore)
add_subdirectory(IbnetMsgrc)
add_subdirectory(IbnetMsgud)
add_subdirectory(IbnetStats)
add_subdirectory(IbnetDx)
add_subdirectory(IbnetSys)
add_subdirectory(MsgrcJNIBinding)
add_subdirectory(MsgrcLoopback)
add_subdirectory(MsgudLoopback)
add_subdirectory(NetworkTest)
add_subdirectory(SocketUdpTest)
add_subdirectory(TimerTest)