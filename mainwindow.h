#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QTime>

#include "FitParse/fitparse.h"
#include "dashboard/dashboard.h"
#include "VideoRenderer/videorenderer.h"
#include "decoder/decoder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:


private slots:

    void Decodered();

private:
    Ui::MainWindow *ui;

    Decoder* decoder = nullptr;
    GL_Image* glImage = nullptr;

    QTimer timer;

protected:


public slots:


    void slotTimeOut();
};
#endif // MAINWINDOW_H
