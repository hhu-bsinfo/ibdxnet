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

project(IbnetCon)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/con/ConnectionManager.cpp
        ${IBNET_SRC_DIR}/ibnet/con/DiscoveryManager.cpp
        ${IBNET_SRC_DIR}/ibnet/con/ExchangeManager.cpp
        ${IBNET_SRC_DIR}/ibnet/con/JobManager.cpp
        ${IBNET_SRC_DIR}/ibnet/con/JobQueue.cpp
        ${IBNET_SRC_DIR}/ibnet/con/NodeConf.cpp
        ${IBNET_SRC_DIR}/ibnet/con/NodeConfArgListReader.cpp
        ${IBNET_SRC_DIR}/ibnet/con/NodeConfStringReader.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} fmt IbnetCore IbnetSys ibverbs pthread)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpic -rdynamic -g -O3 -fno-strict-aliasing")