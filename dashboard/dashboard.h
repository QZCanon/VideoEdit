#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>
#include <QDebug>
#include <math.h>

namespace Ui {
class DashBoard;
}

class DashBoard : public QWidget
{
    Q_OBJECT

public:
    explicit DashBoard(QWidget *parent = 0);
    ~DashBoard();
    void setValue(int value);
    void setMaxValue(int maxValue);//最大值
    void setLargeScaleNum(int largeScaleNum);//大格数
    void setSmallScaleNum(int smallScaleNum);//每两个大格之间又有多少小格

private:
    Ui::DashBoard *ui;

    int value = 0;

    int formWidth = 0, formHeight = 0;
    int pointX = 0, pointY = 0;//圆心坐标
    int largeScaleNum = 12, smallScaleNum = 5;
    int maxValue = 120;
    int beginAngle = 45;//起始角度
    int frontW = 30, frontH = 20;


private:
    void paintEvent(QPaintEvent *);
    void drawDashBoard(QPainter * paint);//画仪表盘
    void drawDivideRule(QPainter * paint);//画刻度尺
    void drawNum(QPainter * paint);//画数值
    void drawPointer(QPainter *paint);//画指针
    void drawCurrentVelocity(QPainter *paint);//画当前速度

private:
    QPoint getCoordinates(int r, double angle);
    QPoint getDrawNumOffset(int frontwidth, int frontHeight, double angle);
};


#endif // DASHBOARD_H
