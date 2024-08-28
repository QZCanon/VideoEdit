#ifndef TIME_H
#define TIME_H

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>

#include <iomanip>  // For std::put_time
#include <string>

namespace Canon::Time {

    std::string format_time(const std::tm& time);

    std::string now();
}

#endif // TIME_H
