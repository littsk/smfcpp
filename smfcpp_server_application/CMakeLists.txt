cmake_minimum_required(VERSION 3.5)

project(smfcpp_server_application)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

file(GLOB SRC_LIST src/*.cpp)
file(GLOB INC_DIRS ../*/include)
file(GLOB LINK_DIRS ../*/build)

link_directories(${LINK_DIRS})

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIRS})
target_link_libraries(${PROJECT_NAME} smfcpp smfcpp_tcp smfcpp_tool)

add_executable(uart_server uart_server.cpp)
target_include_directories(uart_server PUBLIC ${INC_DIRS})
target_link_libraries(uart_server smfcpp smfcpp_server_application)

add_executable(camera_server camera_server.cpp)
target_include_directories(camera_server PUBLIC ${INC_DIRS})
target_link_libraries(camera_server smfcpp smfcpp_server_application)

