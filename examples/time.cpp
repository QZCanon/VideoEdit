#include <QTimer>
#include <QDateTime>
#include <QDebug>

class PreciseTimer : public QObject {
    Q_OBJECT
public:
    PreciseTimer(QObject *parent = nullptr) : QObject(parent), timer(new QTimer(this)) {
        connect(timer, &QTimer::timeout, this, &PreciseTimer::onTimeout);
    }

    void start(int intervalMicroseconds) {
        targetTime = QDateTime::currentMSecsSinceEpoch() + intervalMicroseconds / 1000.0;
        timer->start();
    }

public slots:
    void onTimeout() {
        if (QDateTime::currentMSecsSinceEpoch() < targetTime) {
            timer->.start((targetTime - QDateTime::currentMSecsSinceEpoch()) * 1000);
            return;
        }
        timer->stop();
        qDebug() << "Timer triggered at" << QDateTime::currentMSecsSinceEpoch() << "ms";
        // 执行定时任务
    }

private:
    QTimer *timer;
    qint64 targetTime;
};

// 使用
PreciseTimer *timer = new PreciseTimer();
timer->start(1000000); // 1000000 微秒后触发
