#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <vector>

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
    uint16_t speed;
    uint16_t temperature;
    double position_lat;
    double position_long;
    double grade;
    uint32_t distance;
    uint64_t timeStamp;
} StopWatchMessage;

using StopWatchList = std::vector<Canon::StopWatchMessage>;

}

#endif // TYPES_H
