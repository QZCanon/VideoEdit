#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QTime>
#include <QMenuBar>

#include "FitParse/fitparse.h"
#include "dashboard/dashboard.h"
#include "VideoRenderer/videorenderer.h"
#include "decoder/video_hw_decoder.h"
#include "FitParse/listener.h"
#include "SyncData/syncdata.h"
#include "task_runner/task_runner.hpp"
#include "decoder/audioplayer.h"

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
    void InitComponent();
    void InitFitParse();
    void RepaintComponent(const QSize& size);
    void GetFrameTask();

private slots:
    void SpeedCallback(int);
    void on_restart_clicked();
    void on_keyFrame_clicked();
    void on_play_clicked();

protected:
     void resizeEvent(QResizeEvent* e) override;

signals:
     void Paint();

public slots:
    void PaintFrame();

private:
    Ui::MainWindow *ui;
    QWidget* paintPlane{nullptr};

    TaskRunner* runner{nullptr};
    TaskSPtr m_paintVideoTask = nullptr;

    HwDecoder*    decoder     = nullptr;
    GL_Image*   glImage     = nullptr;
    DashBoard*  dashBoard   = nullptr;
    FitParse*   fitParse    = nullptr;
    Listener*   fitListener = nullptr;
    SyncData*   syncData;
    AudioPlayer* m_audioPalyer{nullptr};


    QSize softwareWinSize;  // 窗口大小
    QSize menuBarSize;      // 菜单栏大小
    QSize paintWinSize;     // 渲染窗口大小
    QSize statusBarSize;    // 状态栏大小
    QSize dashBoardSize = {200, 200};

    const uint16_t MENU_BAR_HEIGHT   = 20;
    const uint16_t STATUS_BAR_HEIGHT = 20;
    uint16_t dashBoardwidth          = 200;
    uint16_t dashBoardHeight         = 200;

#if defined(Q_OS_MAC)
    // std::string m_fileName = "/Users/qinzhou/workspace/test/input_file.mp4";
    std::string m_fileName = "/Users/qinzhou/workspace/test/DJI_20240820194031_0041_D.MP4";
    // std::string m_fileName = "/Users/qinzhou/workspace/test/dage.mp4";
    // std::string m_fileName = "/Users/qinzhou/workspace/test/第054话.mkv";
#elif defined(Q_OS_WIN)
    std::string m_fileName = "F:/DJI_20240908112154_0023_D.MP4";
#endif

};
#endif // MAINWINDOW_H
