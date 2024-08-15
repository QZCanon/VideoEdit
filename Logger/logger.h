#ifndef LOGGER_H
#define LOGGER_H

#include <QtLogging>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
};

class Logger {
public:
    static Logger* GetInstance()
    {
        static Logger logger;
        return &logger;
    }

    void SetLogLevel(LogLevel l) { level = l; }
    LogLevel GetLogLevel() { return level; }

private:
    Logger()  = default;
    ~Logger() = default;

private:
    LogLevel level = DEBUG;
};

// #define LOG_DEBUG() ( \
//     if (Logger::GetInstance->GetLogLevel() <= LogLevel::DEBUG) \
//         qDebug() << "[DEBUG][" <<  __FILE_NAME__ << "][line:" << __LINE__ << "][" << __FUNCTION__ << "()|")

#define LOG_DEBUG() ( \
        qDebug() << "[DEBUG][" <<  __FILE_NAME__ << "][line:" << __LINE__ << "][" << __FUNCTION__ << "()|")

#endif // LOGGER_H
