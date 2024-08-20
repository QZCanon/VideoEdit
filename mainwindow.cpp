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

    glImage = new GL_Image(ui->widget);
    glImage->setFixedSize(ui->widget->size());
    decoder = new Decoder;

    connect(decoder, SIGNAL(DecoderSendAVFrame(AVFrame*)), glImage, SLOT(AVFrameSlot(AVFrame*)));
    connect(decoder, SIGNAL(DecoderIsFinish()), this, SLOT(Decodered()));

    decoder->DoWork();

    connect(&timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    timer.setTimerType(Qt::PreciseTimer);
    // timer.start(10);

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
    // static uint i = 0;

    // char imageName[100];
    // // 需要修改为自己的图片路径
    // sprintf(imageName, "/Users/qinzhou/workspace/test/test.jpg", i++%4);
    // std::string imageName = "/Users/qinzhou/workspace/test/test.jpg";
    // QImage image(imageName.c_str());
    // QImage rgba = image.rgbSwapped(); //qimage加载的颜色通道顺序和opengl显示的颜色通道顺序不一致,调换R通道和B通道
    // glImage->setImageData(rgba.bits(), rgba.width(), rgba.height());
    glImage->repaint(); //窗口重绘，repaint会调用paintEvent函数，paintEvent会调用paintGL函数实现重绘

}


