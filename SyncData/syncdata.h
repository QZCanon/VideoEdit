#ifndef SYNCDATA_H
#define SYNCDATA_H

#include <QObject>

#include <thread>

extern "C" {
#include <libavformat/avformat.h>
}

class SyncData : public QObject
{
    Q_OBJECT
public:
    explicit SyncData(QObject *parent = nullptr);

signals:

private:
    uint64_t m_videoTimestamp;

};

#endif // SYNCDATA_H
