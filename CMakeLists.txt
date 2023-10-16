cmake_minimum_required(VERSION 3.5)
project(smf_test)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-elide-constructors -fPIC")
endif()

# Third Party
add_subdirectory(extern/glog)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11)
set(PUB_LIBS glog::glog)

if(NOT DEFINED COMPILE_CLIENT)
    set(COMPILE_CLIENT OFF)
endif()
if(NOT DEFINED COMPILE_SERVER)
    set(COMPILE_SERVER OFF)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/smfcpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/smfcpp_tcp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/smfcpp_tool)
if(COMPILE_SERVER)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/smfcpp_server_application)
endif()
if(COMPILE_CLIENT)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/smfcpp_client_application)
    # Python Extention
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/smfpy)
endif()