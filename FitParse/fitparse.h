#ifndef FITPARSE_H
#define FITPARSE_H

#include <QDebug>
#include <fstream>

#include "FitSDK/fit_decode.hpp"
#include "FitSDK/fit_mesg_broadcaster.hpp"
#include "FitSDK/fit_developer_field_description.hpp"
#include "FitParse/listener.h"
#include "core/types.h"

class FitParse : public QObject
{
    Q_OBJECT
public:
    explicit FitParse(QObject *parent = nullptr);
    FitParse(std::string& fileName_) : fileName(fileName_) {}

private:
    void readfile();

signals:

private:
    fit::Decode          decode;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener             listener;
    std::fstream         file;
    const std::string    fileName;
    Canon::FitMessage    fitMessage;
};



#endif // FITPARSE_H
