#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    fitParse = std::make_shared<FitParse>();

//    qDebug().setVerbosity(7);

//    qDebug() << "debug";
//    qInfo() << "info";
//    qWarning() << "warning";
//    qCritical() << "qCritical";

}

MainWindow::~MainWindow()
{
    delete ui;
}

