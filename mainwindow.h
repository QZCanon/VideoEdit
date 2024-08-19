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
#include "dashboard/dashboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void InitWindows();
    void InitUIComponent();
    void InitMedia();
    void InitFitParse();
    void InitDashBoard();

private slots:

private:
    Ui::MainWindow *ui;
    QHBoxLayout* layout;

    std::shared_ptr<FitParse>     m_fitParse{nullptr};      // fit文件解析

    std::shared_ptr<QVideoWidget> m_pPlayerWidget{nullptr}; // 视频显示组件
    std::shared_ptr<QAudioOutput> m_audio{nullptr};         // 音频播放器
    std::shared_ptr<QMediaPlayer> m_pPlayer{nullptr};       // 媒体播放器类
    bool                          m_bReLoad;                // 已经载入还没设置进度条最大值
    float                         m_scale{0.0};             // 进度条与媒体文件长度计算比例（100 / duration）

    std::shared_ptr<DashBoard>    m_dashBoard{nullptr};

protected:
    void moveEvent(QMoveEvent *) override;

public slots:
    void OnSetMediaFile(void);                           // 选择文件导入
    void OnDurationChanged(qint64);                      // 信号表示内容的持续时间已更改为时长，以毫秒表示，进度条 的最大值和最小值
    void OnPositionChanged(qint64);                      // 当前播放位置
    void OnMediaStateChanged(QMediaPlayer::MediaStatus); // Player对象的状态已经改变

};
#endif // MAINWINDOW_H
