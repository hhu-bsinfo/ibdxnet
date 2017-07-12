project(JNIIbdxnet)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

include_directories(/usr/lib/jvm/java-8-openjdk/include/)
include_directories(/usr/lib/jvm/java-8-openjdk-amd64/include/)
include_directories(/usr/lib/jvm/java-8-openjdk/include/linux)
include_directories(/usr/lib/jvm/java-8-openjdk-amd64/include/linux)

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/dx/ConnectionCreator.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/ConnectionHandler.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/DebugThread.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/DiscoveryHandler.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/JNIIbdxnet.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/RecvBufferPool.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/RecvHandler.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/RecvThread.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/SendBuffers.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/SendHandler.cpp
        ${IBNET_SRC_DIR}/ibnet/dx/SendThread.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} IbnetCore)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpic")