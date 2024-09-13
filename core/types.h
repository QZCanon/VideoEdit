#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <vector>
#include <QByteArray>

#include "Logger/logger.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace Canon {

/*
 * 码表数据，当前每种数据只支持一个
 */
typedef struct StopWatchMessage_ {
    StopWatchMessage_()
      : isValid(false)
      , speed(0)
      , temperature(0)
      , position_lat(0.0)
      , position_long(0.0)
      , grade(0.0)
      , distance(0)
      , timeStamp(0)
    {}
    bool isValid;
    double speed; // m/s
    double temperature;
    double position_lat;
    double position_long;
    double grade;
    double distance; // m
    uint64_t timeStamp;
} StopWatchMessage;

using StopWatchList = std::vector<Canon::StopWatchMessage>;

// 音频信息
typedef struct AudioData_ {
    bool operator==(const AudioData_& data) {
        return this->timestamp == data.timestamp;
    }
    uint64_t timestamp;
    QByteArray data;
} AudioData;

// 视频帧完整信息
typedef struct VideoFrame_ {
    VideoFrame_()
        : frame(nullptr)
        , stream_index(0)
        , packet_index(0)
        , pts(0)
        , width(0)
        , height(0)
    {}
    uint8_t*   frame;
    int        stream_index;
    int        packet_index;
    int64_t    pts;
    int        width;
    int        height;
    AVRational timeBase;
    double     duration; // 当前帧的持续时间
} VideoFrame;

// 视频关键帧信息
typedef struct VideoKeyFrame_ {
    VideoKeyFrame_()
        : timestamp(0)
        , posOffset(0)
    {}

    bool operator==(const VideoKeyFrame_& val) {
        return (val.posOffset == this->posOffset &&
                val.timestamp == this->timestamp);
    }

    uint64_t timestamp;
    uint64_t posOffset;
} VideoKeyFrame;

}

#define PRINT_MSGS(MSGS)                                      \
    LOG_DEBUG() << "speed: "          << MSGS.speed           \
                << "m/s, temprature: "    << MSGS.temperature \
                << ", position_lat: "  << MSGS.position_lat   \
                << ", position_long: " << MSGS.position_long  \
                << ", grade: "         << MSGS.grade          \
                << ", distance: "      << MSGS.distance       \
                << "m, timeStamp: "     << MSGS.timeStamp


#endif // TYPES_H
