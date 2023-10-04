configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in config.h)

find_package(yaml-cpp REQUIRED)

pybind11_add_module(C_uart uart.cpp)
target_include_directories(C_uart PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(C_uart PUBLIC ${PUB_LIBS} smfcpp smfcpp_tcp smfcpp_tool smfcpp_client_application yaml-cpp)
