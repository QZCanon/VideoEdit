#ifndef MYSLIDE_H
#define MYSLIDE_H

#include <QWidget>

#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QDebug>
//#include <windows.h>

namespace Ui {
class MySlide;
}

class MySlide : public QWidget
{
    Q_OBJECT

public:
    explicit MySlide(QWidget *parent = 0);
    ~MySlide();

    void setValue(const int value);
    void setButtonHeight(int h);
    void setButtonAutoHide(bool isAutoHide);//button 自动隐藏

private:
    Ui::MySlide *ui;


signals:
    void currentProgressValue(int);


private:
    int progressValue = 0;
    int lineHeight = 4;
    int progressSumPix = 0;
    int btnX = 0, btnY = 0;
    int currentY = 0;
    QPoint pressPoint;
    bool isPressBtn = false;
    bool saveVal = false;
    int buttonHeight = 14;
    int buttonAutoHide = true;
    bool buttonIsVisibe = false;

private:

protected:
    void mouseMoveEvent(QMouseEvent *event);         //移动
    void mousePressEvent(QMouseEvent *event);        //单击
    void mouseReleaseEvent(QMouseEvent *event);//抬起
    void paintEvent(QPaintEvent * );
};

#endif // MYSLIDE_H
