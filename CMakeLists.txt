#cmake_minimum_required(VERSION 3.26)
cmake_minimum_required(VERSION 3.15)
project(push_stream)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
# 设置运行架构
set(CMAKE_OSX_ARCHITECTURES "arm64")

#set(LIBYUV_DIR /Users/yaohua/Downloads/libyuv) # libyuv的安装目录
#include_directories(${LIBYUV_DIR}/include) # 头文件路径
#link_directories(${LIBYUV_DIR}/lib) # 链接库路径

#file(GLOB SRCS
#        audio *.cpp
#        video *.cpp
#
#
#)
set(SOURCE_FILES
        main.cpp
)


add_subdirectory(rtmp_stream)
add_subdirectory(test)

add_executable(main ${SOURCE_FILES})







