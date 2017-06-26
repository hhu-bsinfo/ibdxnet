project(JNIIbnet)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${IBNET_LIBS_DIR})
include_directories(${IBNET_SRC_DIR})

include_directories(/usr/lib/jvm/java-8-openjdk/include/)
include_directories(/usr/lib/jvm/java-8-openjdk-amd64/include/)
include_directories(/usr/lib/jvm/java-8-openjdk/include/linux)
include_directories(/usr/lib/jvm/java-8-openjdk-amd64/include/linux)

set(SOURCE_FILES
        ${IBNET_SRC_DIR}/ibnet/jni/Callbacks.cpp
        ${IBNET_SRC_DIR}/ibnet/jni/DebugThread.cpp
        ${IBNET_SRC_DIR}/ibnet/jni/JNIIbnet.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} IbnetMsg)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpic")