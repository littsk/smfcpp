project(smfcpp_tcp)

file(GLOB SRC_LIST src/*.cpp)
add_library(smfcpp_tcp STATIC ${SRC_LIST})
target_include_directories(smfcpp_tcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/smfcpp/include)
target_link_libraries(smfcpp_tcp smfcpp)

add_executable(tcp_server tcp_server.cpp)
target_include_directories(tcp_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tcp_server smfcpp_tcp smfcpp)

add_executable(tcp_client tcp_client.cpp)
target_include_directories(tcp_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tcp_client smfcpp_tcp smfcpp)