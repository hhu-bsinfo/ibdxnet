project(SocketUdpTest)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/sys/test/SocketUdpTest.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} IbnetSys)