#ifndef DEFINES_H
#define DEFINES_H

#include <functional>
#include "types.h"

using StopWatchMessageCallback = std::function<void(Canon::StopWatchMessage&)>;

#endif // DEFINES_H
