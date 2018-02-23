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

project(MsgrcJNIBinding)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

include_directories(/usr/lib/jvm/java-8-openjdk/include/)
include_directories(/usr/lib/jvm/java-8-openjdk-amd64/include/)
include_directories(/usr/lib/jvm/java-8-openjdk/include/linux)
include_directories(/usr/lib/jvm/java-8-openjdk-amd64/include/linux)

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/msgrc/jni/MsgrcJNIBinding.cpp
        ${IBNET_SRC_DIR}/ibnet/msgrc/jni/MsgrcJNIBindingCallbackHandler.cpp
        ${IBNET_SRC_DIR}/ibnet/msgrc/jni/MsgrcJNISystem.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} IbnetMsgrc IbnetDx IbnetStats IbnetCore IbnetSys)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic -g -O3 -fno-strict-aliasing")