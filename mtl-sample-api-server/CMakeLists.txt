cmake_minimum_required(VERSION 3.4.1)
project(server CXX)

set (CMAKE_CXX_STANDARD 14)
set (CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# include IXWebSocket library
include_directories(${PROJECT_SOURCE_DIR}/../IXWebSocket/ixwebsocket)

# include nlohmann library
include_directories(../IXWebSocket/third_party)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*")
add_executable(server ${SOURCE_FILES})

#linking executable to ixwebsocket and its dependencies
target_link_libraries(server ${CMAKE_CURRENT_SOURCE_DIR}/../IXWebSocket/build/libixwebsocket.a pthread z)
