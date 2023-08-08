project(smfcpp_tool)

file(GLOB SRC_LIST src/*.cpp)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC include)


