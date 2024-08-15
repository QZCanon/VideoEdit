#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

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
    QVideoWidget* m_pPlayerWidget{nullptr}; // 视频显示组件
    QAudioOutput* m_audio{nullptr};         // 音频播放器
    QMediaPlayer* m_pPlayer;                // 媒体播放器类
    bool          m_bReLoad;                // 已经载入还没设置进度条最大值
    float         m_scale{0.0};

public slots:
    void OnSetMediaFile(void);                           // 选择文件导入
    void OnDurationChanged(qint64);                      // 信号表示内容的持续时间已更改为时长，以毫秒表示，进度条 的最大值和最小值
    void OnPositionChanged(qint64);                      // 当前播放位置
    void OnMediaStateChanged(QMediaPlayer::MediaStatus); // Player对象的状态已经改变

};
#endif // MAINWINDOW_H
