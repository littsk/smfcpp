project(smfcpp_client_application)

find_package(OpenCV REQUIRED)

find_path(FFMPEG_INCLUDE_DIRS libavcodec/avcodec.h)
set(FFMPEG_LIB_NAMES avcodec avformat avutil swscale)
foreach(LIB ${FFMPEG_LIB_NAMES})
    find_library(FFMPEG_${LIB}_LIBRARY NAMES ${LIB})
    list(APPEND FFMPEG_LIBS ${FFMPEG_${LIB}_LIBRARY})
endforeach()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64|AMD64|amd64")
    find_library(MindVison_LIBS MVSDK HINTS ${CMAKE_SOURCE_DIR}/smfcpp_tool/x64)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm|ARM")
    find_library(MindVison_LIBS MVSDK HINTS ${CMAKE_SOURCE_DIR}/smfcpp_tool/arm64)
endif()

file(GLOB SRC_LIST src/*.cpp)
file(GLOB INC_DIRS ${CMAKE_SOURCE_DIR}/*/include)

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIRS} ${OpenCV_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${FFMPEG_LIBS} smfcpp smfcpp_tool smfcpp_tcp ${MindVison_LIBS})

add_executable(camera_client camera_client.cpp)
target_link_libraries(camera_client ${PROJECT_NAME})