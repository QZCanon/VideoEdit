#include "fitparse.h"
#include "Logger/logger.h"

FitParse::FitParse(QObject *parent) : QObject(parent)
{
    listener.SetFitMessageCallback(std::bind(&FitParse::MessageCallback,
                                             this,
                                             std::placeholders::_1));

}

void FitParse::ReadFitFile(const std::string& fileName)
{
    // stopWatchMsgList.clear(); // 先清空队列

    std::fstream file;
    file.open(fileName, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
       LOG_DEBUG() << "Error opening file " << fileName;
    }

    if (!decode.CheckIntegrity(file)) {
       LOG_DEBUG() << "FIT file integrity failed.\nAttempting to decode...";
    }

    // add listener
    mesgBroadcaster.AddListener((fit::FileIdMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::UserProfileMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::MonitoringMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::DeviceInfoMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::RecordMesgListener&)listener);
    mesgBroadcaster.AddListener((fit::MesgListener &)listener);

    // read
    try {
       decode.Read(&file, &mesgBroadcaster, &mesgBroadcaster, &listener);
    } catch (const fit::RuntimeException& e) {
       LOG_DEBUG() << "Exception decoding file: " << e.what();
    } catch (...) {
       LOG_DEBUG() << "Exception decoding file";
    }
}

void FitParse::MessageCallback(Canon::StopWatchMessage &msg)
{
    emit SendStopWatchMsg(msg);
    // LOG_DEBUG() << "msg fit is rec";
    // {
    //     std::unique_lock<std::mutex> lock(mutex);
    //     stopWatchMsgList.push_back(msg);
        // PRINT_MSGS(msg);
        // LOG_DEBUG() << ", list size: " << stopWatchMsgList.size();
    // }
}
