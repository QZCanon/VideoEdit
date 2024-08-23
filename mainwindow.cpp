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

    // GL_Image renderer;
    // renderer.show();

    qRegisterMetaType< AVFrame* >("AVFrame*");//注册类型

    glImage = new GL_Image(ui->widget);
    glImage->setFixedSize(ui->widget->size());
    glImage->Start();
    decoder = new Decoder;

    connect(decoder, SIGNAL(DecoderSendAVFrame(AVFrame*)), glImage, SLOT(AVFrameSlot(AVFrame*)), Qt::BlockingQueuedConnection);
    connect(decoder, SIGNAL(DecoderIsFinish()), this, SLOT(Decodered()));
    decoder->Init();

    connect(&timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    timer.setTimerType(Qt::PreciseTimer);
    auto fps = decoder->GetFileFPS();
    int time = 1000 / fps;
    LOG_DEBUG() << "time: " << time;
    timer.start(time);

    QPushButton * button = new QPushButton("cli", ui->widget);
    button->setGeometry(100,100,100,30);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Decodered()
{
    timer.start(10);
}


// 主界面开启定时器，在界面循环显示4个方向的图片
void MainWindow::slotTimeOut()
{
    if (decoder->BufferIsEmpty()) {
        return;
    }
    glImage->SetFrame(decoder->GetFrame());
    glImage->repaint(); //窗口重绘，repaint会调用paintEvent函数，paintEvent会调用paintGL函数实现重绘

}


