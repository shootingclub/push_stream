

#include "rtmp_stream.h"

namespace stream {


    static int read_u8(FILE *fp, unsigned int *u8) {
        unsigned int tmp;
        if (fread(&tmp, 1, 1, fp) != 1) {
            printf("Failed to read_u8!\n");
            return -1;
        }

        *u8 = tmp & 0xFF;

        return 0;
    }

/*
 ...| | | | | ...
 ...x1, x2, x3, x4, x5, xxxxx...
 ...|x1|x2|x3| | ...
 ...|x3|x2|x1| | ...
 */
    static int read_u24(FILE *fp, unsigned int *u24) {
        unsigned int tmp;
        if (fread(&tmp, 1, 3, fp) != 3) {
            printf("Failed to read_u24!\n");
            return -1;
        }
        *u24 = ((tmp >> 16) & 0xFF) | ((tmp << 16) & 0xFF0000) | (tmp & 0xFF00);

        return 0;
    }

    static int read_u32(FILE *fp, unsigned int *u32) {
        unsigned int tmp;
        if (fread(&tmp, 1, 4, fp) != 4) {
            printf("Failed to read_u32!\n");
            return -1;
        }
        *u32 = ((tmp >> 24) & 0xFF) | ((tmp >> 8) & 0xFF00) | \
           ((tmp << 8) & 0xFF0000) | ((tmp << 24) & 0xFF000000);

        return 0;
    }

    static int read_ts(FILE *fp, unsigned int *ts) {
        unsigned int tmp;
        if (fread(&tmp, 1, 4, fp) != 4) {
            printf("Failed to read_ts!\n");
            return -1;
        }

        *ts = ((tmp >> 16) & 0xFF) | ((tmp << 16) & 0xFF0000) | (tmp & 0xFF00) | (tmp & 0xFF000000);

        return 0;
    }


    /*
* 一共有 9 个字节
* 1-3, signature: ‘F’ ‘L’ ‘V’
* 4, version : 1
* 5, 0-5位 保留，必须是 0
* 5, 6位，是否有音频 Tag
* 5, 7位，保留，必须是 0
* 5, 8位，表示是否有视频 Tag
* 6-9, Header 的大小，必须是 9
*/
    FILE *pushStream::open_flv(char *flv_name) {
        FILE *fp = nullptr;

        fp = fopen(flv_name, "rb");
        if (!fp) {
            printf("Failed to open flv: %s", flv_name);
            return nullptr;
        }

        fseek(fp, 9, SEEK_SET); //跳过 9 字节的 FLV Header
        fseek(fp, 4, SEEK_CUR); //跳过 4 字节的PreTagSize

        return fp;
    }

    //connect rtmp server
    RTMP *pushStream::conect_rtmp_server(char *rtmpaddr) {
        RTMP *rtmp = nullptr;
        //1. 创建RTMP对象,并进行初始化
        rtmp = RTMP_Alloc();
        if (!rtmp) {
            printf("NO Memory, Failed to alloc RTMP object!\n");
            goto __ERROR;
        }

        RTMP_Init(rtmp);
        //2.先设置RTMP服务地址，以及设置连接超时间
        rtmp->Link.timeout = 10;
        RTMP_SetupURL(rtmp, rtmpaddr);

        //3. 设置是推流还是拉流
        //如果设置了该开关，就是推流(publish)，如果未设置就是拉流（play)
        RTMP_EnableWrite(rtmp);

        //4. 建立连接
        if (!RTMP_Connect(rtmp, nullptr)) {
            printf("Failed to Connect RTMP Server!\n");
            goto __ERROR;
        }

        //5. create stream
        RTMP_ConnectStream(rtmp, 0);

        return rtmp;

        __ERROR:

        //释放资源
        if (rtmp) {
            RTMP_Close(rtmp);
            RTMP_Free(rtmp);
        }

        return nullptr;
    }

    //向流媒体服务器推流
    void pushStream::send_data(FILE *fp, RTMP *rtmp) {

        //1. 创建 RTMPPacket 对象
        RTMPPacket *packet = NULL;
        packet = alloc_packet();
        packet->m_nInfoField2 = rtmp->m_stream_id;

        unsigned int pre_ts = 0;

        while (1) {
            //2.从flv文件中读取数据
            if (read_data(fp, &packet)) {
                printf("over!\n");
                break;
            }

            //3. 判断RTMP连接是否正常
            if (!RTMP_IsConnected(rtmp)) {
                printf("Disconnect....\n");
                break;
            }

            unsigned int diff = packet->m_nTimeStamp - pre_ts;
            usleep(diff * 1000);

            //4. 发送数据
            RTMP_SendPacket(rtmp, packet, 0);

            pre_ts = packet->m_nTimeStamp;
        }

        return;
    }

    //分配RTMPPacket空间
    RTMPPacket *pushStream::alloc_packet() {

        RTMPPacket *pack;
        pack = (RTMPPacket *) malloc(sizeof(RTMPPacket));
        if (!pack) {
            printf("No Memory, Failed to alloc RTMPPacket!\n");
            return nullptr;
        }

        if (!RTMPPacket_Alloc(pack, 64 * 1024)) {
            //
        }

        RTMPPacket_Reset(pack);

        pack->m_hasAbsTimestamp = 0;
        pack->m_nChannel = 0x4;

        return pack;
    }

    /**
     * @param[in] fp : flv file
    * @param[out] packet the data from flv
    * @return 0: success, -1: failure
    */
    int pushStream::read_data(FILE *fp, RTMPPacket **packet) {

        /*
         * tag header
         * 第一个字节 TT（Tag Type）, 0x08 音频，0x09 视频， 0x12 script
         * 2-4, Tag body 的长度， PreTagSize - Tag Header size
         * 5-7, 时间戳，单位是毫秒; script 它的时间戳是0
         * 第8个字节，扩展时间戳。真正时间戳结格 [扩展，时间戳] 一共是4字节。
         * 9-11, streamID, 0
         */

        /*
         * flv
         * flv header(9), tagpresize, tag(header+data), tagpresize
         */
        int ret = -1;
        size_t datasize = 0;

        unsigned int tt;
        unsigned int tag_data_size;
        unsigned int ts;
        unsigned int streamid;
        unsigned int tag_pre_size;

        if (read_u8(fp, &tt)) {
            goto __ERROR;
        }

        if (read_u24(fp, &tag_data_size)) {
            goto __ERROR;
        }

        if (read_ts(fp, &ts)) {
            goto __ERROR;
        }

        if (read_u24(fp, &streamid)) {
            goto __ERROR;
        }

        printf("tag header, ts: %u, tt: %d, datasize:%d \n", ts, tt, tag_data_size);

        datasize = fread((*packet)->m_body, 1, tag_data_size, fp);
        if (tag_data_size != datasize) {
            printf("Failed to read tag body from flv, (datasize=%zu:tds=%d)\n",
                   datasize,
                   tag_data_size);
            goto __ERROR;
        }

        //设置pakcet数据
        (*packet)->m_headerType = RTMP_PACKET_SIZE_LARGE;
        (*packet)->m_nTimeStamp = ts;
        (*packet)->m_packetType = tt;
        (*packet)->m_nBodySize = tag_data_size;

        read_u32(fp, &tag_pre_size);

        ret = 0;

        __ERROR:
        return ret;

    }

    void pushStream::publish_stream(char *flv, char *rtmpaddr) {

        //1. 读 flv 文件
        FILE *fp = open_flv(flv);

        //2. 连接 RTMP 服务器
        RTMP *rtmp = conect_rtmp_server(rtmpaddr);

        //3. publish audio/video data
        send_data(fp, rtmp);

        //4. release rtmp

        return;
    }
}

