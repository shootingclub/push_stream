

#define __STDC_CONSTANT_MACROS
#ifndef VIDEO_LEARN_PUSHSTREAM_H
#define VIDEO_LEARN_PUSHSTREAM_H

#include <cstdio>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
#include "rtmpdump/2.4-20151223_3/include/librtmp/rtmp.h"
}


namespace stream {
    class pushStream {

    private:
        RTMPPacket *alloc_packet();

        int read_data(FILE *fp, RTMPPacket **packet);

        FILE *open_flv(char *flv_name);

        RTMP *conect_rtmp_server(char *rtmpaddr);

        void send_data(FILE *fp, RTMP *rtmp);

    public:
        void publish_stream(char *flv, char *rtmpaddr);
    };
}


#endif //VIDEO_LEARN_PUSHSTREAM_H

