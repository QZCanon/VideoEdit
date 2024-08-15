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

    fitParse = std::make_shared<FitParse>();
    ui->BtnStop->setEnabled(false);// 停止：默认不可用

    m_bReLoad = true;
    setWindowTitle("播放器");
    setWindowIcon(QIcon(":/icon/icon.webp"));// 图标
    setFixedSize(800,600);//设置固定大小

    // 1.放置播放窗口
    m_pPlayer       = new QMediaPlayer;//媒体播放器类
    m_pPlayerWidget = new QVideoWidget;// 视频显示组件窗口
    m_audio         = new QAudioOutput;

    m_audio->setVolume(50);
    m_pPlayer->setVideoOutput(m_pPlayerWidget);//视频输出:设置视频输出，允许用户将视频渲染到自定义的视频输出设备上。
    m_pPlayer->setAudioOutput(m_audio);
    ui->verticalLayout->addWidget(m_pPlayerWidget);//将视频显示组件窗口添加到QVBoxLayout垂直布局

    // 设置视频小部件是否自动填充背景。true则视频小部件将自动填充背景，以便在视频播放期间保持视觉效果。
    m_pPlayerWidget->setAutoFillBackground(false);

    // 2.界面美化
    QPalette qplte;// 调色板
    qplte.setColor(QPalette::Window, QColor(0,0,0));// 透明
    m_pPlayerWidget->setPalette(qplte);// 设置窗口部件的调色板

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnSetMediaFile()));
    connect(ui->BtnPlay,    SIGNAL(clicked()), m_pPlayer, SLOT(play()));// 对QMediaPlayer媒体播放器操作
    connect(ui->BtnStop,    SIGNAL(clicked()), m_pPlayer, SLOT(stop()));// 对QMediaPlayer媒体播放器操作

    connect(m_pPlayer,  SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this,       SLOT(OnMediaStateChanged(QMediaPlayer::MediaStatus)));
    connect(m_pPlayer,  SIGNAL(positionChanged(qint64)), this, SLOT(OnSlider(qint64)));
    connect(m_pPlayer,  SIGNAL(durationChanged(qint64)), this, SLOT(OnDurationChanged(qint64))); // 时长最大值和最小值
    connect(m_pPlayer,  SIGNAL(positionChanged(qint64)), this, SLOT(OnPositionChanged(qint64))); // 当前播放位置

    // init test
    const std::string fileName = "/Users/qinzhou/workspace/test/input_file.mp4";
    m_pPlayer->setSource(QUrl::fromLocalFile(fileName.c_str()));// 设置媒体信息
    setWindowTitle(fileName.c_str());
}

MainWindow::~MainWindow()
{
    delete m_pPlayer;
    delete m_pPlayerWidget;
    delete ui;
}

void MainWindow::OnSetMediaFile()
{
    QFileDialog dialog(this);//文件选择窗口
    dialog.setFileMode(QFileDialog::AnyFile);//设置文件模式(文件/文件夹)：任意文件，无论是否存在
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();// 存所有选择的文件
    }

    if(!fileNames.empty()) {
        LOG_DEBUG() << "文件名: " << fileNames[0];
        m_pPlayer->setSource(QUrl::fromLocalFile(fileNames[0]));// 设置媒体信息
        m_bReLoad = true;
        ui->myslide->setValue(0);//进度条数字在0(开始位置)
        setWindowTitle(fileNames[0]);
    }
}

// 信号表示内容的持续时间已更改为时长，以毫秒表示，进度条 的最大值和最小值
void MainWindow::OnDurationChanged(qint64 i64Duration)
{
    if(i64Duration > 0 && m_bReLoad) { // 最大值>0并且 已经载入还没设置进度条最大值
        // ui->slider->setRange(0, i64Duration);
        ui->minTime->setText("00:00:00");
        QTime time = QTime::fromMSecsSinceStartOfDay(i64Duration);
        m_scale = (float)100 / i64Duration;
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
    if(QMediaPlayer::MediaStatus::LoadedMedia == status) { // 停止
        ui->BtnPlay->setEnabled(true);
        ui->BtnStop->setEnabled(false);
        ui->myslide->setEnabled(false);
    } else if(QMediaPlayer::MediaStatus::BufferedMedia == status)// 播放
    {
        ui->BtnPlay->setEnabled(false);
        ui->BtnStop->setEnabled(true);
        ui->myslide->setEnabled(true);
    }
}


