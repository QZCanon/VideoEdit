#ifndef FITPARSE_H
#define FITPARSE_H

#include <QDebug>
#include <fstream>
#include <vector>
#include <mutex>

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
    FitParse(std::string& fileName_) : fileName(fileName_) {}

private:
    void ReadFitFile();
    void MessageCallback(Canon::FitMessage& fitMsg);

signals:

private:
    fit::Decode          decode;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener             listener;
    const std::string    fileName;

    std::mutex                     mutex;
    std::vector<Canon::FitMessage> fitMsgList;

};



#endif // FITPARSE_H
