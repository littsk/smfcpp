cmake_minimum_required(VERSION 3.5)

project(smfcpp_tcp)

# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

link_directories(/home/taozewei/smartfarm_frame/smfcpp/build)

file(GLOB SRC_LIST src/*.cpp)
add_library(smfcpp_tcp STATIC ${SRC_LIST})
target_include_directories(smfcpp_tcp PUBLIC ${CMAKE_SOURCE_DIR}/include /home/taozewei/smartfarm_frame/smfcpp/include)
target_link_libraries(smfcpp_tcp smfcpp)

add_executable(tcp_server tcp_server.cpp)
target_include_directories(tcp_server PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tcp_server smfcpp_tcp smfcpp)

add_executable(tcp_client tcp_client.cpp)
target_include_directories(tcp_client PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tcp_client smfcpp_tcp smfcpp)