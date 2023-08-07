cmake_minimum_required(VERSION 3.5)
project(smf_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-elide-constructors")
endif()

add_subdirectory(smfcpp)

add_executable(smf_test main.cpp)
target_link_libraries(smf_test PUBLIC base)
target_include_directories(smf_test PUBLIC tool/include)