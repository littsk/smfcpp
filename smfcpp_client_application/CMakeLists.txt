cmake_minimum_required(VERSION 3.5)

project(smfcpp_client_application)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

file(GLOB SRC_LIST src/*.cpp)
file(GLOB INC_DIRS ../*/include)

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIRS})
# target_link_libraries(${PROJECT_NAME})
