#ifndef FITPARSE_H
#define FITPARSE_H

#include <QDebug>
#include <QObject>
#include <fstream>
#include <mutex>
#include <thread>

#include "FitSDK/fit_decode.hpp"
#include "FitSDK/fit_mesg_broadcaster.hpp"
#include "FitSDK/fit_developer_field_description.hpp"
#include "FitParse/listener.h"
#include "core/types.h"
#include "core/defines.h"

class FitParse : public QObject
{
    Q_OBJECT
public:
    explicit FitParse(QObject *parent = nullptr);

    void ReadFitFile(const std::string& fileName);

private:
    void MessageCallback(Canon::StopWatchMessage& fitMsg);

signals:

private:
    fit::Decode          decode;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener             listener;
    bool isStart = false;
    std::string fitFileName;

    std::thread th;
    std::condition_variable cond_;
    std::mutex mutex_;

    std::mutex           mutex;
    Canon::StopWatchList stopWatchMsgList;
    void DoWork();
};



#endif // FITPARSE_H
