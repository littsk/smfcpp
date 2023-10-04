project(smfcpp)

file(GLOB SRC_LIST src/*.cpp)
add_library(smfcpp STATIC ${SRC_LIST})
target_include_directories(smfcpp PUBLIC include)

add_executable(my_test main.cpp)
target_link_libraries(my_test smfcpp)
