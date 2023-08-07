cmake_minimum_required(VERSION 3.5)

project(smfcpp)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")


file(GLOB SRC_LIST src/*.cpp)
add_library(smfcpp STATIC ${SRC_LIST})
target_include_directories(smfcpp PUBLIC include)

add_executable(my_test main.cpp)
target_link_libraries(my_test smfcpp)

message("SRC_LIST = ${SRC_LIST}")
