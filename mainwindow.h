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
#include <QMenuBar>

#include "FitParse/fitparse.h"
#include "dashboard/dashboard.h"
#include "VideoRenderer/videorenderer.h"
#include "decoder/decoder.h"
#include "FitParse/listener.h"

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
    void RepaintComponent(const QSize& size);

private slots:

    void Decodered();

protected:
     void resizeEvent(QResizeEvent* e) override;

public slots:
    void slotTimeOut();


private:
    Ui::MainWindow *ui;
    QWidget* paintPlane{nullptr};

    Decoder* decoder = nullptr;
    GL_Image* glImage = nullptr;
    DashBoard* dashBoard = nullptr;
    QTimer timer;
    FitParse* fitParse = nullptr;
    Listener* fitListener = nullptr;

    QSize softwareWinSize;  // 窗口大小
    QSize menuBarSize;      // 菜单栏大小
    QSize paintWinSize;     // 渲染窗口大小
    QSize statusBarSize;    // 状态栏大小
    QSize dashBoardSize = {200, 200};

    const uint16_t MENU_BAR_HEIGHT   = 20;
    const uint16_t STATUS_BAR_HEIGHT = 20;
    uint16_t dashBoardwidth          = 200;
    uint16_t dashBoardHeight         = 200;

    void InitComponent();
    void InitFitParse();
};
#endif // MAINWINDOW_H
