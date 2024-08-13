#ifndef FITPARSE_H
#define FITPARSE_H

#include <QDebug>
#include <fstream>

#include "FitSDK/fit_decode.hpp"
#include "FitSDK/fit_mesg_broadcaster.hpp"
#include "FitSDK/fit_developer_field_description.hpp"
#include "FitParse/listener.h"

class FitParse : public QObject
{
    Q_OBJECT
public:
    explicit FitParse(QObject *parent = nullptr);

signals:

private:
    fit::Decode          decode;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener             listener;
    std::fstream         file;
};



#endif // FITPARSE_H
