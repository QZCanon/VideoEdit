#include "fitparse.h"

FitParse::FitParse(QObject *parent) : QObject(parent)
{
    listener.SetFitMessageCallback(std::bind(&FitParse::MessageCallback,
                                             this,
                                             std::placeholders::_1));
}

void FitParse::ReadFitFile(const std::string& fileName)
{
    stopWatchMsgList.clear(); // 先清空队列

    std::fstream file;
    file.open(fileName, std::ios::in | std::ios::binary);

    // open
    if (!file.is_open())
    {
       // qDebug() << "Error opening file " << fileName;
    }

    if (!decode.CheckIntegrity(file))
    {
       printf("FIT file integrity failed.\nAttempting to decode...\n");
    }

    // add listener
    mesgBroadcaster.AddListener((fit::FileIdMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::UserProfileMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::MonitoringMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::DeviceInfoMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::RecordMesgListener&)listener);
    mesgBroadcaster.AddListener((fit::MesgListener &)listener);

    // read
    try
    {
       decode.Read(&file, &mesgBroadcaster, &mesgBroadcaster, &listener);
    }
    catch (const fit::RuntimeException& e)
    {
       qDebug() << "Exception decoding file: " << e.what();
    }
    catch (...)
    {
       qDebug() << "Exception decoding file";
    }
}

void FitParse::MessageCallback(Canon::StopWatchMessage &msg)
{
    {
        std::unique_lock<std::mutex> lock(mutex);
        stopWatchMsgList.push_back(msg);
    }
}
