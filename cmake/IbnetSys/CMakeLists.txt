project(IbnetSys)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/sys/AddressIPV4.cpp
        ${IBNET_SRC_DIR}/ibnet/sys/Debug.cpp
        ${IBNET_SRC_DIR}/ibnet/sys/Logger.cpp
        ${IBNET_SRC_DIR}/ibnet/sys/Network.cpp
        ${IBNET_SRC_DIR}/ibnet/sys/Random.cpp
        ${IBNET_SRC_DIR}/ibnet/sys/SocketUDP.cpp
        ${IBNET_SRC_DIR}/ibnet/sys/StringUtils.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} dl dw)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -rdynamic -g -fpic -O3")