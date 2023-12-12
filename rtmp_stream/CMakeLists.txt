project(rtmp_stream)
set(HEADER_FILES
        rtmp_stream.h
)

set(SOURCE_FILES
        rtmp_stream.cpp

)

add_library(rtmp_stream STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(rtmp_stream PUBLIC .)
# 添加ffmpeg库
#include_directories(/opt/homebrew/Cellar/rtmpdump/2.4-20151223_3/include)
#link_directories(/opt/homebrew/Cellar/rtmpdump/2.4-20151223_3/lib)
#
#target_link_libraries(pushStream
#        rtmp
#)