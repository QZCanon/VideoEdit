#ifndef FITPARSE_H
#define FITPARSE_H

#include <QDebug>
#include <QObject>

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

private:
    void MessageCallback(Canon::StopWatchMessage&);

signals:
    void SendStopWatchMsg(Canon::StopWatchMessage&);

private:
    fit::Decode          decode;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener             listener;
};



#endif // FITPARSE_H
