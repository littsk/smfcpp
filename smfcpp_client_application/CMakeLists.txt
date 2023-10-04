project(smfcpp_client_application)

configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in config.h)

find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Curses)

find_path(FFMPEG_INCLUDE_DIRS libavcodec/avcodec.h)
set(FFMPEG_LIB_NAMES avcodec avformat avutil swscale)
foreach(LIB ${FFMPEG_LIB_NAMES})
    find_library(FFMPEG_${LIB}_LIBRARY NAMES ${LIB})
    list(APPEND FFMPEG_LIBS ${FFMPEG_${LIB}_LIBRARY})
endforeach()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64|AMD64|amd64")
    find_library(MindVison_LIBS MVSDK HINTS ${CMAKE_SOURCE_DIR}/smfcpp_tool/x64)
    file(GLOB HikVision_LIBS_1 ${CMAKE_SOURCE_DIR}/smfcpp_tool/hik_lib/*.so)
    # file(GLOB HikVision_LIBS_2 ${CMAKE_SOURCE_DIR}/smfcpp_tool/hik_lib/HCNetSDKCom/*.so)
    # 合并两次匹配的结果到同一个变量
    list(APPEND HikVision_LIBS ${HikVision_LIBS_1})
    # list(APPEND HikVision_LIBS ${HikVision_LIBS_2})
    
    message("HikVision_LIBS: ${HikVision_LIBS}")

elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm|ARM")
    message(WARNING "arm platform cannot support hikvision camera now")
    find_library(MindVison_LIBS MVSDK HINTS ${CMAKE_SOURCE_DIR}/smfcpp_tool/arm64)
endif()

file(GLOB SRC_LIST src/*.cpp)

file(GLOB INC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND INC_DIRS ${OpenCV_INCLUDE_DIRS})
list(APPEND INC_DIRS ${FFMPEG_INCLUDE_DIRS})
list(APPEND INC_DIRS ${CMAKE_CURRENT_BINARY_DIR})

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} ${PUB_LIBS} ${OpenCV_LIBS} ${FFMPEG_LIBS} ${HikVision_LIBS} smfcpp smfcpp_tool smfcpp_tcp ${MindVison_LIBS} yaml-cpp)

add_executable(video_survailence video_survailence.cpp)
target_link_libraries(video_survailence ${PROJECT_NAME})

add_executable(sensor_survailence sensor_survailence.cpp)
target_link_libraries(sensor_survailence ${PROJECT_NAME})

add_executable(hik_push hik_push.cpp)
target_include_directories(hik_push PUBLIC ${INC_DIRS})
target_link_libraries(hik_push ${OpenCV_LIBS} ${FFMPEG_LIBS})

add_executable(test test.cpp)
target_link_libraries(test smfcpp smfcpp_tool)

# add_executable(uart_device_controler uart_device_controler.cpp)
# target_include_directories(uart_device_controler PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# target_link_libraries(uart_device_controler smfcpp smfcpp_tool yaml-cpp ${CURSES_LIBRARIES})