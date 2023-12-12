#include "gtest/gtest.h"
#include <iostream>
#include "rtmp_stream.h"


TEST(PushStreamTestSuite, PushStream) {
    char *flv = "lizi.flv";
    char *rtmpaddr = "rtmp://localhost/live/room";
    stream::pushStream pushStream;
    pushStream.publish_stream(flv, rtmpaddr);
}