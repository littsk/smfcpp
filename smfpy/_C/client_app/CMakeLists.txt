configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in config.h)

find_package(yaml-cpp REQUIRED)

# Create an extension module
# add_library(uart MODULE uart.cpp)
# target_include_directories(uart PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
pybind11_add_module(uart uart.cpp)
target_include_directories(uart PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(uart PUBLIC smfcpp_tool yaml-cpp)
