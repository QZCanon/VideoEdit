#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPixmap>

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QTime>

#include "FitParse/fitparse.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    Ui::MainWindow *ui;

    std::shared_ptr<FitParse> fitParse{nullptr};

    // VideoPlayer *player{nullptr};
    QVideoWidget *m_pPlayerWidget;// 视频显示组件
    QMediaPlayer * m_pPlayer;//媒体播放器类


public slots:
    void    OnSetMediaFile(void);//载入
    void    OnSlider(qint64); // 信号内容的位置已更改为位置，以毫秒表示，进度条也要变
    void    OnDurationChanged(qint64);// 信号表示内容的持续时间已更改为时长，以毫秒表示，进度条 的最大值和最小值
    // void    OnStateChanged(QMediaPlayer::State);// Player对象的状态已经改变



};
#endif // MAINWINDOW_H
