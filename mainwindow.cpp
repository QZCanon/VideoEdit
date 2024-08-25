#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/logger.h"

#include <QFileDialog>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitFitParse();
}

MainWindow::~MainWindow()
{
    if (paintPlane) delete paintPlane;
    if (glImage)    delete glImage;
    if (dashBoard)  delete dashBoard;
    delete ui;
}

void MainWindow::InitFitParse()
{
    fitParse = new FitParse;
    fitParse->ReadFitFile("/Users/qinzhou/workspace/qt/VideoEdit_test/MAGENE_C416_2024-08-11_194425_907388_1723381873.fit");
}

void MainWindow::InitComponent()
{
    softwareWinSize = this->size();
    menuBarSize     = {this->size().width(), MENU_BAR_HEIGHT};
    paintWinSize    = {this->size().width(), this->size().height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT};
    statusBarSize   = {this->size().width(), STATUS_BAR_HEIGHT};
    dashBoardSize   = {dashBoardwidth, dashBoardHeight};


    qRegisterMetaType<AVFrame*>("AVFrame*");//注册类型

    softwareWinSize = this->size();


    paintPlane = new QWidget(this);
    paintPlane->setGeometry(0,
                            MENU_BAR_HEIGHT,
                            softwareWinSize.width(),
                            softwareWinSize.height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    paintPlane->setStyleSheet("background-color: black;");



    glImage = new GL_Image(paintPlane);
    glImage->setFixedSize(paintWinSize);

    decoder = new Decoder;

    connect(decoder, SIGNAL(DecoderSendAVFrame(AVFrame*)),
            glImage, SLOT(AVFrameSlot(AVFrame*)), Qt::BlockingQueuedConnection);
    connect(decoder, SIGNAL(DecoderIsFinish()), this, SLOT(Decodered()));
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

void MainWindow::Decodered()
{
    // timer.start(10);
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
    if (glImage) {
        glImage->SetFrame(decoder->GetFrame());
        glImage->repaint(); //窗口重绘，repaint会调用paintEvent函数，paintEvent会调用paintGL函数实现重绘
    }

}


