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


    InitWindows();
    InitMedia();
    InitUIComponent();
    InitFitParse();
    InitDashBoard();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitWindows()
{
    setWindowTitle("播放器");
    // setWindowIcon(QIcon(":/icon/icon.webp"));// 图标
    // setFixedSize(800,600);//设置固定大小
}

void MainWindow::InitMedia()
{
    m_pPlayer       = std::make_shared<QMediaPlayer>();
    m_pPlayerWidget = std::make_shared<QVideoWidget>(this);
    m_audio         = std::make_shared<QAudioOutput>();

    m_audio->setVolume(50);
    m_pPlayer->setVideoOutput(m_pPlayerWidget.get());
    m_pPlayer->setAudioOutput(m_audio.get());

    m_pPlayerWidget->setAutoFillBackground(false);
    // m_pPlayerWidget->setGeometry(0, 0, wGeometry.width(), wGeometry.height());
    m_pPlayerWidget->setGeometry(0, 0, 200, 200);

    QPalette qplte;                                  // 调色板
    qplte.setColor(QPalette::Window, QColor(0,0,0)); // 透明
    m_pPlayerWidget->setPalette(qplte);              // 设置窗口部件的调色板

    connect(m_pPlayer.get(),  SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this,             SLOT(OnMediaStateChanged(QMediaPlayer::MediaStatus)));
    connect(m_pPlayer.get(),  SIGNAL(durationChanged(qint64)), this, SLOT(OnDurationChanged(qint64))); // 时长最大值和最小值
    connect(m_pPlayer.get(),  SIGNAL(positionChanged(qint64)), this, SLOT(OnPositionChanged(qint64))); // 当前播放位置

    // test
    m_bReLoad = true;
    const std::string fileName = "/Users/qinzhou/workspace/test/input_file.mp4";
    m_pPlayer->setSource(QUrl::fromLocalFile(fileName.c_str()));
    setWindowTitle(fileName.c_str());
}

void MainWindow::InitUIComponent()
{
    ui->BtnStop->setEnabled(false);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnSetMediaFile()));
    connect(ui->BtnPlay,    SIGNAL(clicked()), m_pPlayer.get(), SLOT(play()));
    connect(ui->BtnStop,    SIGNAL(clicked()), m_pPlayer.get(), SLOT(stop()));
}

void MainWindow::InitFitParse()
{

}

void MainWindow::InitDashBoard()
{
    m_dashBoard = std::make_shared<DashBoard>();
    m_dashBoard->setFixedSize(300, 300);
    // m_dashBoard.get()->setGeometry(50, 50, 100, 100);
    m_dashBoard.get()->move(0, 0);
    m_dashBoard.get()->show();

    // QLabel *label = new QLabel("qwe");
    // label->setFixedSize(100, 30);
    // label->move(20, 20);
    // label->show();
    // label->setWindowOpacity(0.2);
}

void MainWindow::OnSetMediaFile()
{
    QFileDialog dialog(this);                   //文件选择窗口
    dialog.setFileMode(QFileDialog::AnyFile);   //设置文件模式(文件/文件夹)：任意文件，无论是否存在
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();     // 存所有选择的文件
    }

    if(!fileNames.empty()) {
        LOG_DEBUG() << "文件名: " << fileNames[0];
        m_pPlayer->setSource(QUrl::fromLocalFile(fileNames[0]));// 设置媒体信息
        m_bReLoad = true;
        ui->myslide->setValue(0);                               //进度条数字在0(开始位置)
        setWindowTitle(fileNames[0]);
    }
}

// 信号表示内容的持续时间已更改为时长，以毫秒表示，进度条 的最大值和最小值
void MainWindow::OnDurationChanged(qint64 i64Duration)
{
    if(i64Duration > 0 && m_bReLoad) {
        QTime time = QTime::fromMSecsSinceStartOfDay(i64Duration);
        m_scale = (float)100 / i64Duration;
        ui->minTime->setText("00:00:00");
        ui->maxTime->setText(time.toString("hh:mm:ss"));
        m_bReLoad = false; // 不能再设置进度条最大值
    }
}

void MainWindow::OnPositionChanged(qint64 position)
{
    // LOG_DEBUG() << "silder pos: " << position;
    QTime time = QTime::fromMSecsSinceStartOfDay(position);
    ui->minTime->setText(time.toString("hh:mm:ss"));
    ui->myslide->setValue((int)(position * m_scale));
}

// Player对象的状态已经改变
void MainWindow::OnMediaStateChanged(QMediaPlayer::MediaStatus status)
{
    if(QMediaPlayer::MediaStatus::LoadedMedia == status || // 加载完成
        QMediaPlayer::MediaStatus::EndOfMedia == status) { // 播放结束
        ui->BtnPlay->setEnabled(true);
        ui->BtnStop->setEnabled(false);
        ui->myslide->setEnabled(false);
        ui->myslide->setValue(0);
        ui->minTime->setText("00:00:00");
    } else if(QMediaPlayer::MediaStatus::BufferedMedia == status) { // 正在播放
        ui->BtnPlay->setEnabled(false);
        ui->BtnStop->setEnabled(true);
        ui->myslide->setEnabled(true);
    }
}


void MainWindow::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event); // 确保基类处理默认行为
    // qDebug() << "Window moved to:" << event->pos();
    if (m_dashBoard)
        m_dashBoard.get()->move(event->pos().x(), event->pos().y());
}


