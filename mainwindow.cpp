#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsOpacityEffect>
#include "Logger/logger.h"

#include <QFileDialog>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    runner = new TaskRunner;

    InitComponent();
    InitFitParse();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitFitParse()
{
    syncData = new SyncData(runner);
    fitParse = new FitParse;

    connect(fitParse, SIGNAL(SendStopWatchMsg(Canon::StopWatchMessage&)),
            syncData, SLOT(AcceptStopWatchData(Canon::StopWatchMessage&)));
    connect(syncData, SIGNAL(SpeedSignal(int)),
            this,     SLOT(SpeedCallback(int)),
            Qt::ConnectionType::BlockingQueuedConnection);

#if defined(Q_OS_MAC)
    fitParse->ReadFitFile("/Users/qinzhou/workspace/qt/VideoEdit_test"
                          "/MAGENE_C416_2024-08-11_194425_907388_1723381873.fit");
#elif defined(Q_OS_WIN)
    fitParse->ReadFitFile("F:/0830/0830.fit");
#endif

    syncData->SetImageTimestame(0);
    syncData->Start();
}

void MainWindow::InitComponent()
{
    softwareWinSize = this->size();
    menuBarSize     = {this->size().width(), MENU_BAR_HEIGHT};
    paintWinSize    = {this->size().width(), this->size().height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT};
    statusBarSize   = {this->size().width(), STATUS_BAR_HEIGHT};
    dashBoardSize   = {dashBoardwidth, dashBoardHeight};
    qRegisterMetaType<AVFrame*>("AVFrame*");//注册类型

    // 初始化渲染面板
    paintPlane = new QWidget(this);
    paintPlane->setGeometry(0,
                            MENU_BAR_HEIGHT * 2,
                            softwareWinSize.width(),
                            softwareWinSize.height() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    paintPlane->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    paintPlane->setWindowFlags(Qt::Widget);  // 确保是普通窗口

    // 初始化播放器
    m_playerImpl = new PlayerImpl(m_fileName, paintPlane);
    m_playerImpl->SetTaskRunner(runner);
    // m_playerImpl->PlayAudio();
    m_playerImpl->PlayVideo();

    // 初始化仪表盘
    dashBoard = new DashBoard();
    dashBoard->setMaxValue(50);
    int ww = paintPlane->width();
    int wh = paintPlane->height();
    int w = 200, h = 200;
    dashBoard->setGeometry(ww - w, wh - h, dashBoardSize.width(), dashBoardSize.height());

    // m_playerImpl->SetDashBoard(dashBoard);
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
}

void MainWindow::SpeedCallback(int speed)
{
    // LOG_DEBUG() << "speed : " << speed;
    if (dashBoard) {
        dashBoard->setValue(speed);
    }
}

