#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPixmap>

#include "FitParse/fitparse.h"
#include "VideoPlayer/VideoPlayer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    std::shared_ptr<FitParse> fitParse{nullptr};

    VideoPlayer *player{nullptr};
};
#endif // MAINWINDOW_H
