#include "rtmp_stream/rtmp_stream.h"

int main(int argc, const char *argv[]) {
    char *flv = "lizi.flv";
    char *rtmpaddr = "rtmp://localhost/live/room";
    stream::pushStream pushStream;
    pushStream.publish_stream(flv, rtmpaddr);
}
