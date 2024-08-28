#include "syncdata.h"

SyncData::SyncData(QObject *parent)
    : QObject{parent}
{
    if (!task) {
        task = CREATE_TASK_OCJECT();
    }
    if (!runner) {
        runner = TaskRunner::GetInstance();
    }

    task->SetTaskFunc(std::bind(&SyncData::Sync, this));
}

void SyncData::Sync()
{
    if (m_update.load() == false) {
        return;
    }
}

