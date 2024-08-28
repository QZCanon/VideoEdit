#ifndef SYNCDATA_H
#define SYNCDATA_H

#include <QObject>

#include "task_runner/task_runner.hpp"
#include "Logger/logger.h"
#include "core/types.h"

extern "C" {
#include <libavformat/avformat.h>
}

class SyncData : public QObject
{
    Q_OBJECT
public:
    explicit SyncData(QObject *parent = nullptr);

    void SetImageTimestame(uint64_t t) {
        m_videoTimestamp = t;
        m_update         = true;
    }

    void Sync();
signals:

public slots:
    void AcceptStopWatchData(Canon::StopWatchMessage& msg) {
        stopWatchMsgList.push_back(msg);
        PRINT_MSGS(msg);
    }

private:
    std::atomic<uint64_t> m_videoTimestamp;
    std::atomic<bool>     m_update{false};
    TaskRunner::TaskRSPtr runner{nullptr};
    TaskSPtr              task{nullptr};
    Canon::StopWatchList  stopWatchMsgList;

};

#endif // SYNCDATA_H
