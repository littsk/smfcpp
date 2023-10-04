pybind11_add_module(C_init init.cpp)
target_link_libraries(C_init PUBLIC smfcpp_tool)