#cmake_minimum_required(VERSION 3.26)
project(test)
# 添加模块，gtest模块
add_subdirectory(lib)
#
file(GLOB SRCS *.cpp)
# adding the Google_Tests_run target
add_executable(Google_Tests_run
        ${SRCS}
)
target_link_libraries(Google_Tests_run
        rtmp_stream
)

target_link_libraries(Google_Tests_run gtest gtest_main)