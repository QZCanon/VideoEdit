#include "syncdata.h"

SyncData::SyncData(TaskRunner* runner, QObject *parent)
    : QObject{parent}
{
    m_runner = runner;
    if (!m_task) {
        m_task = CREATE_TASK_OCJECT();
    }
    m_task->SetTaskFunc(std::bind(&SyncData::Sync, this));
}

SyncData::~SyncData()
{
}

void SyncData::Sync()
{
    if (m_update.load() == false) {
        ExitTask();
        return;
    }

    // if (m_currentIndex >= m_stopWatchMsgList.size()) {
    //     LOG_DEBUG() << "The current index is 0, need reset";
    //     m_currentIndex = 0;
    // }

    const auto& data = m_stopWatchMsgList[m_currentIndex];

    if (data.timeStamp > m_videoTimestamp.load()) { // 视频时间戳早于码表，不可用
        m_task->CancleTask(); // 退出task
        m_taskIsAdded = true;
        return;
    }

    if (data.timeStamp == m_videoTimestamp.load()) {
        if (m_task) {
            m_task->CancleTask(); // 同步上，退出task
            m_taskIsAdded = true;
        }
        LOG_DEBUG() << "current speed: " << data.speed;
    } else {
        m_currentIndex++;
    }
}

