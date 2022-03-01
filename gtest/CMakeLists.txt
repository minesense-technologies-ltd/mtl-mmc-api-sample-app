cmake_minimum_required(VERSION 3.4.1)
project(client_unittest CXX)

set (CMAKE_CXX_STANDARD 14)
set (CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# include IXWebSocket library
include_directories(${PROJECT_SOURCE_DIR}/../IXWebSocket/ixwebsocket)

# include nlohmann library
include_directories(${PROJECT_SOURCE_DIR}/../IXWebSocket/third_party) 

#include headers
include_directories(${PROJECT_SOURCE_DIR}/../mtl-sample-api-client/include)

file(GLOB SOURCE_FILES "${PROJECT_SOURCE_DIR}/../mtl-sample-api-client/src/Mtl*.c*")
add_executable(client_unittest ${PROJECT_SOURCE_DIR}/client_unit_test.cpp ${SOURCE_FILES})

target_link_libraries(client_unittest ${PROJECT_SOURCE_DIR}/../IXWebSocket/build/libixwebsocket.a pthread z gtest_main gtest)
