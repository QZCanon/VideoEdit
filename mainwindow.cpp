#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/logger.h"

#include <QFileDialog>
#include <QTime>
#include "core/types.h"

#include "SDL2/SDL.h"

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

void MainWindow::InitSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOG_DEBUG() << "SDL init is fail";
        return ;
    }

    // SDL_Window* window = SDL_CreateWindowFrom((void *)winId());
    // 准备音频数据
    QByteArray audioData; // 假设这里填充了音频数据
    SDL_AudioSpec desiredSpec, obtainedSpec;
    SDL_AudioCVT cvt;
    SDL_AudioDeviceID deviceId;

    // 设置期望的音频格式
    desiredSpec.freq = 44100; // 采样率
    desiredSpec.format = AUDIO_S16SYS; // 音频格式，这里以16位系统音频为例
    desiredSpec.channels = 2; // 声道数
    desiredSpec.samples = 4096; // 每次回调填充的样本数
    desiredSpec.callback = NULL; // 使用默认音频回调

    // 打开音频设备
    deviceId = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &obtainedSpec, 0);
    if (deviceId == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        SDL_Quit();
        return ;
    }

    // 将音频数据复制到SDL的音频缓冲区
    // 注意：这里需要确保audioData的格式与desiredSpec匹配
    Uint8* buffer = new Uint8[audioData.size()];
    memcpy(buffer, audioData.constData(), audioData.size());

    // 开始播放音频
    SDL_QueueAudio(deviceId, buffer, audioData.size());
    SDL_PauseAudioDevice(deviceId, 0);

    // 等待一段时间，或者执行其他任务，直到音频播放完成
    SDL_Delay(5000); // 假设音频长度足够播放5秒

    // 清理资源
    delete[] buffer;
    SDL_CloseAudioDevice(deviceId);
    SDL_Quit();

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
    // glImage->repaint();
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
    return;
    m_audioPalyer->Replay();
    decoder->Restart();
}

void MainWindow::on_keyFrame_clicked()
{
    return;
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
    // m_audioPalyer->Play();
}

