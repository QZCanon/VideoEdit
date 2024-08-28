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
    explicit SyncData(TaskRunner* runner = nullptr, QObject *parent = nullptr);
    ~SyncData();

    void SetImageTimestame(uint64_t t) {
        m_videoTimestamp = t;
        m_update         = true;
    }

    void Start() {
        if (m_task && m_runner) {
            m_runner->AddTask(m_task);
        }
    }

private:
    void Sync();

signals:

public slots:
    void AcceptStopWatchData(Canon::StopWatchMessage& msg) {
        m_stopWatchMsgList.push_back(msg);
        PRINT_MSGS(msg);
    }

private:
    std::atomic<uint64_t> m_videoTimestamp;
    std::atomic<bool>     m_update{false};
    TaskRunner*           m_runner{nullptr};
    TaskSPtr              m_task{nullptr};
    Canon::StopWatchList  m_stopWatchMsgList;
    uint64_t              m_currentIndex = 0;

};

#endif // SYNCDATA_H
