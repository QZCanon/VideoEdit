#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "Logger/logger.h"
#include <thread>
#include "VideoPlayer/VideoPlayer.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}
std::thread th;
int labelWidth = 640;
int labelHeight = 480;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    fitParse = std::make_shared<FitParse>();

    player = new VideoPlayer("/Users/qinzhou/workspace/test/input_file.mp4", ui->label);
    player->start();

}

MainWindow::~MainWindow()
{

    player->stopPlayback();
    player->wait();

    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
}
