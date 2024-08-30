#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/logger.h"

#include <QFileDialog>
#include <QTime>
#include "core/types.h"
#include "core/time.hpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    runner = new TaskRunner;

    InitFitParse();
    InitComponent();

}

MainWindow::~MainWindow()
{
    // if (paintPlane) delete paintPlane;
    // if (glImage)    delete glImage;
    // if (dashBoard)  delete dashBoard;
    delete ui;
}

void MainWindow::InitFitParse()
{
    syncData = new SyncData(runner);
    fitParse = new FitParse;

    // 创建对象之后，先连接信号槽，当前fit文件解析是在主线程
    connect(fitParse, SIGNAL(SendStopWatchMsg(Canon::StopWatchMessage&)),
            syncData, SLOT(AcceptStopWatchData(Canon::StopWatchMessage&)));

#if defined(Q_OS_MAC)
    fitParse->ReadFitFile("/Users/qinzhou/workspace/qt/VideoEdit_test/MAGENE_C416_2024-08-11_194425_907388_1723381873.fit");
#elif defined(Q_OS_WIN)
    fitParse->ReadFitFile("F:/MAGENE_C416_2024-08-11_194425_907388_1723381873.fit");
#endif

}

void MainWindow::InitComponent()
{
    softwareWinSize = this->size();
    menuBarSize     = {this->size().width(), MENU_BAR_HEIGHT};
    paintWinSize    = {this->size().width(), this->size().height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT};
    statusBarSize   = {this->size().width(), STATUS_BAR_HEIGHT};
    dashBoardSize   = {dashBoardwidth, dashBoardHeight};
    qRegisterMetaType<AVFrame*>("AVFrame*");//注册类型
    paintPlane = new QWidget(this);
    paintPlane->setGeometry(0,
                            MENU_BAR_HEIGHT,
                            softwareWinSize.width(),
                            softwareWinSize.height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    paintPlane->setStyleSheet("background-color: black;");

    glImage = new GL_Image(paintPlane);
    glImage->setFixedSize(paintWinSize);
    decoder = new Decoder;

    decoder->Init();

    connect(&timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    timer.setTimerType(Qt::PreciseTimer);
    auto fps = decoder->GetFileFPS();
    int time = 1000 / fps;
    LOG_DEBUG() << "time: " << time;
    timer.start(time);

    dashBoard = new DashBoard(paintPlane);
    int ww = paintPlane->width();
    int wh = paintPlane->height();
    int w = 200, h = 200;
    dashBoard->setGeometry(ww - w, wh - h, dashBoardSize.width(), dashBoardSize.height());
}

void MainWindow::RepaintComponent(const QSize& size)
{
    softwareWinSize = size;
    menuBarSize     = {size.width(), MENU_BAR_HEIGHT};
    paintWinSize    = {size.width(), size.height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT};
    statusBarSize   = {size.width(), STATUS_BAR_HEIGHT};
    dashBoardSize   = {dashBoardwidth, dashBoardHeight};

    if (paintPlane) {
        paintPlane->resize(paintWinSize);
    }
    if (dashBoard) {
        dashBoard->setGeometry(paintWinSize.width() - dashBoardSize.width(),
                               paintWinSize.height() - dashBoardSize.height(),
                               dashBoardSize.width(),
                               dashBoardSize.height());
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    RepaintComponent(e->size());
    // glImage->setFixedSize(paintWinSize);
}


// 主界面开启定时器，在界面循环显示4个方向的图片
void MainWindow::slotTimeOut()
{
    if (decoder->BufferIsEmpty()) {
        return;
    }
    if (glImage &&  !glImage->BePainting()) {
        auto* frame = decoder->GetFrame();

        // LOG_DEBUG() << "pts: " << frame->pts;
        // LOG_DEBUG() << "dts: " << frame->pkt_dts;
        // LOG_DEBUG() << "base time  den: " << frame->time_base.den
        //             << ", num: " << frame->time_base.num;
        // int64_t pts_in_us = frame->pts; // 假设这是原始的 PTS 值，单位是微秒
        // double pts_in_seconds = av_q2d(frame->time_base) * pts_in_us; // 转换为秒
        // // LOG_DEBUG() << "pts_in_seconds: " << (int)pts_in_seconds;
        // if (syncData) {
        //     syncData->SetImageTimestame((uint64_t)pts_in_seconds);
        //     // syncData->Start();
        // }
        glImage->SetFrame(frame);
        glImage->repaint();
    }
}


void MainWindow::on_add_task_clicked()
{
}


void MainWindow::on_cancle_task_clicked()
{
}

