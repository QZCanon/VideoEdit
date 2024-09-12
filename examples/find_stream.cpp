#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// 哈希表结构定义（简化版）
typedef struct KeyFrameHash {
    int64_t timestamp; // 关键帧时间戳
    int64_t offset;    // 关键帧在文件中的偏移量
    struct KeyFrameHash *next; // 链表中的下一个节点
} KeyFrameHash;

// 哈希表大小
#define HASH_TABLE_SIZE 1024

// 简单的哈希函数
unsigned int hash(int64_t timestamp) {
    return (unsigned int)(timestamp % HASH_TABLE_SIZE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return -1;
    }

    const char *filename = argv[1];
    AVFormatContext *format_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    AVCodec *codec = NULL;
    AVPacket packet;
    int video_stream_index = -1;
    int ret;

    // 注册所有的编解码器
    av_register_all();

    // 打开视频文件
    if (avformat_open_input(&format_ctx, filename, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return -1;
    }

    // 获取流信息
    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }

    // 查找视频流
    video_stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (video_stream_index < 0) {
        fprintf(stderr, "Could not find a video stream\n");
        return -1;
    }

    // 获取视频流的解码器上下文
    codec_ctx = format_ctx->streams[video_stream_index]->codec;
    codec_ctx->codec_id = codec->id;

    // 打开解码器
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }

    // 创建哈希表
    KeyFrameHash *hash_table[HASH_TABLE_SIZE] = {0};

    // 读取数据包
    while (av_read_frame(format_ctx, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            // 检查是否为关键帧
            if (packet.flags & AV_PKT_FLAG_KEY) {
                // 计算哈希值
                unsigned int hash_key = hash(packet.dts);
                // 创建新的哈希表节点
                KeyFrameHash *entry = (KeyFrameHash *)malloc(sizeof(KeyFrameHash));
                entry->timestamp = packet.dts;
                entry->offset = packet.pos;
                entry->next = hash_table[hash_key];
                hash_table[hash_key] = entry;
            }
        }
        av_packet_unref(&packet);
    }

    // 打印哈希表中的关键帧信息
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        KeyFrameHash *entry = hash_table[i];
        while (entry) {
            printf("Timestamp: %"PRId64", Offset: %"PRId64"\n", entry->timestamp, entry->offset);
            KeyFrameHash *temp = entry;
            entry = entry->next;
            free(temp);
        }
    }

    // 清理
    avcodec_close(codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}