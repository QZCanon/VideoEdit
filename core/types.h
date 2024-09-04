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
 * 当前每种数据只支持一个
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

}

#define PRINT_MSGS(MSGS)                                     \
    LOG_DEBUG() << "speed: "          << MSGS.speed           \
                << "m/s, temprature: "    << MSGS.temperature \
                << ", position_lat: "  << MSGS.position_lat   \
                << ", position_long: " << MSGS.position_long  \
                << ", grade: "         << MSGS.grade          \
                << ", distance: "      << MSGS.distance       \
                << "m, timeStamp: "     << MSGS.timeStamp

typedef struct FrameInfo_ {
    FrameInfo_()
        : frame(nullptr)
        , stream_index(0)
        , packet_index(0)
    {}
    AVFrame *frame;
    int stream_index;
    int packet_index;
} FrameInfo;

typedef struct AudioData_ {
    uint64_t timestamp;
    QByteArray data;
} AudioData;

#endif // TYPES_H
