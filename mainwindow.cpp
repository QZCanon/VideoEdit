#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/logger.h"

#include <QFileDialog>
#include <QTime>
#include "core/types.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    runner = new TaskRunner;

    m_audioPalyer = new AudioPlayer(m_fileName, runner);
    InitComponent();
    InitFitParse();

    connect(this, &MainWindow::Paint, this, &MainWindow::PaintFrame, Qt::AutoConnection);

    m_paintVideoTask = CREATE_TASK_OCJECT();
    uint64_t sleep = 1000000 / decoder->GetFileFPS();
    LOG_DEBUG() << "task sleep: " << sleep;
    m_paintVideoTask->SetTaskSleepTime(sleep);
    m_paintVideoTask->SetTaskFunc(std::bind(&MainWindow::GetFrameTask, this));
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
    connect(syncData, SIGNAL(SpeedSignal(int)),
            this,     SLOT(SpeedCallback(int)),
            Qt::ConnectionType::BlockingQueuedConnection);

#if defined(Q_OS_MAC)
    fitParse->ReadFitFile("/Users/qinzhou/workspace/qt/VideoEdit_test/MAGENE_C416_2024-08-11_194425_907388_1723381873.fit");
#elif defined(Q_OS_WIN)
    fitParse->ReadFitFile("F:/0830/0830.fit");
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

    decoder = new HwDecoder(m_fileName);
    decoder->Start();

    // dashBoard = new DashBoard(paintPlane);
    // dashBoard->setMaxValue(50);
    // int ww = paintPlane->width();
    // int wh = paintPlane->height();
    // int w = 200, h = 200;
    // dashBoard->setGeometry(ww - w, wh - h, dashBoardSize.width(), dashBoardSize.height());
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

void MainWindow::GetFrameTask()
{
    decoder->BufferWait(); // 为空时应该wait
    m_paintVideoTask->SetTaskSleepTime(decoder->GetPaintFrameDuration()); // 设置当前帧的持续时长
    if (glImage &&  !glImage->BePainting()) {
        emit Paint();
    }
}

void MainWindow::PaintFrame()
{
    auto frame = decoder->GetFrame();
    if (!frame->frame) {
        return;
    }
    uint64_t timestamp = TimeBase2Timestamp(frame->pts, decoder->GetCreateTime(), frame->timeBase);
    m_audioPalyer->Play(timestamp);
    if (syncData) {
        syncData->SetImageTimestame(timestamp);
        syncData->Start();
    }
    glImage->SetFrame(frame);
    glImage->repaint();
}

void MainWindow::SpeedCallback(int speed)
{
    // LOG_DEBUG() << "speed : " << speed;
    if (dashBoard) {
        dashBoard->setValue(speed);
    }
}


void MainWindow::on_restart_clicked()
{
    m_audioPalyer->Replay();
    decoder->Restart();
}

void MainWindow::on_keyFrame_clicked()
{
    // Canon::VideoKeyFrame keyFrame;
    // keyFrame.posOffset = 70805634;
    // keyFrame.timestamp = 640630;
    // decoder->StartFromKeyFrameAsync(keyFrame);
}


void MainWindow::on_play_clicked()
{
    // auto fps = decoder->GetFileFPS();
    // int time = 1000 / fps;
    // LOG_DEBUG() << "set timer: " << time << "ms";
    // timer.start(time + 1);
    runner->AddTask(m_paintVideoTask);
}

