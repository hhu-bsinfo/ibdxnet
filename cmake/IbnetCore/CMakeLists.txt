project(IbnetCore)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/core/IbCompQueue.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbConnection.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbConnectionCreatorSimple.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbConnectionManager.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbDevice.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbDiscoveryManager.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbNodeConf.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbNodeConfArgListReader.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbProtDom.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbQueuePair.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbRecvQueue.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbSendQueue.cpp
        ${IBNET_SRC_DIR}/ibnet/core/IbSharedRecvQueue.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} IbnetSys ibverbs pthread)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpic -rdynamic -g -O3")