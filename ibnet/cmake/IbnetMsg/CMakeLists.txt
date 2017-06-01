project(IbnetMsg)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/msg/BufferPool.cpp
        ${IBNET_SRC_DIR}/ibnet/msg/ConnectionCreator.cpp
        ${IBNET_SRC_DIR}/ibnet/msg/IbMessageSystem.cpp
        ${IBNET_SRC_DIR}/ibnet/msg/RecvThread.cpp
        ${IBNET_SRC_DIR}/ibnet/msg/SendQueues.cpp
        ${IBNET_SRC_DIR}/ibnet/msg/SendThread.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} IbnetCore IbnetSys ibverbs pthread dl dw)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -rdynamic -g")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpic")