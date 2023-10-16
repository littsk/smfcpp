project(smfcpp_server_application)

configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in config.h)

file(GLOB SRC_LIST src/*.cpp)
file(GLOB INC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND INC_DIRS ${CMAKE_CURRENT_BINARY_DIR})

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIRS})
target_link_libraries(${PROJECT_NAME} smfcpp smfcpp_tcp smfcpp_tool)

add_executable(uart_server uart_server.cpp)
target_include_directories(uart_server PUBLIC ${INC_DIRS})
target_link_libraries(uart_server smfcpp smfcpp_server_application ${PUB_LIBS})
