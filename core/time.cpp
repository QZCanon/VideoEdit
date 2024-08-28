#include "time.hpp"

namespace Canon::Time {

std::string format_time(const std::tm& time) {
    std::ostringstream oss;
    // Use std::put_time to format the time
    oss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string now()
{
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();

    // 转换为time_t类型
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // 转换为localtime结构体
    std::tm* now_tm = std::localtime(&now_time_t);

    // 使用std::put_time格式化输出
    // std::cout << "Current system time: "
    //           << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S")  // 格式化时间
    //           << std::endl;
    // std::cout << format_time(*now_tm) << std::endl;
    std::string s = format_time(*now_tm);

    return s;
}

}
