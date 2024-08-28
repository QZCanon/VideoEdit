#ifndef FITPARSE_H
#define FITPARSE_H

#include <QDebug>
#include <QObject>
#include <fstream>
#include <mutex>

#include "FitSDK/fit_decode.hpp"
#include "FitSDK/fit_mesg_broadcaster.hpp"
#include "FitParse/listener.h"
#include "core/types.h"

class FitParse : public QObject
{
    Q_OBJECT
public:
    explicit FitParse( QObject *parent = nullptr);

    void ReadFitFile(const std::string& fileName);

    size_t Size() {
        return stopWatchMsgList.size();
    }

    auto& at(int i) {
        return stopWatchMsgList.at(i);
    }

private:
    void MessageCallback(Canon::StopWatchMessage& fitMsg);

signals:
    void SendStopWatchMsg(Canon::StopWatchMessage&);

private:
    fit::Decode          decode;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener             listener;

    std::mutex           mutex;
    Canon::StopWatchList stopWatchMsgList;
};



#endif // FITPARSE_H
