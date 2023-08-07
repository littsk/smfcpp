cmake_minimum_required(VERSION 3.5)

project(smfcpp_tool)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

file(GLOB SRC_LIST src/*.cpp)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC include)


