#ifndef SYNCDATA_H
#define SYNCDATA_H

#include <QObject>

#include <thread>

class SyncData : public QObject
{
    Q_OBJECT
public:
    explicit SyncData(QObject *parent = nullptr);

signals:

};

#endif // SYNCDATA_H
