#include "dashboard.h"
#include "ui_dashboard.h"

#include <QGraphicsOpacityEffect>

#define OUTERMOST_WIIDTH 20//仪表盘最外面圆宽度
#define SCALE_WIIDTH 50 //刻度圆环宽度
#define TRIANGLE_WIDTH 16//指针三角形底的宽度
#define CENTER_WIIDTH 80//中心红色最外侧圆直径
#define WHITE_WIDTH 65  //中心圆白色区域有圆直径

DashBoard::DashBoard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DashBoard)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->installEventFilter(this);
}

DashBoard::~DashBoard()
{
    delete ui;
}

void DashBoard::setValue(int value)
{
    if(value >= 0 && value <= maxValue)
    {
        this->value = value;
        repaint();
    }
}

void DashBoard::setMaxValue(int maxValue)
{
    if(maxValue > 0)
    {
        this->maxValue = maxValue;
        repaint();
    }
}

void DashBoard::setLargeScaleNum(int largeScaleNum)
{
    if(largeScaleNum>0)
    {
        this->largeScaleNum = largeScaleNum;
        //        repaint();
    }
}

void DashBoard::setSmallScaleNum(int smallScaleNum)
{
    if(smallScaleNum > 0)
    {
        this->smallScaleNum = smallScaleNum;
        //        repaint();
    }
}

void DashBoard::paintEvent(QPaintEvent *)
{
    formHeight = this->height();
    formWidth = this->width();
    if(formHeight >= formWidth)
        formHeight = formWidth;
    else
        formWidth = formHeight;
    pointX = formWidth/2;
    pointY = formHeight/2;
    QPainter  paint(this);
    paint.setRenderHint(QPainter::Antialiasing);

    drawDashBoard(&paint);
    drawDivideRule(&paint);
    drawNum(&paint);
    drawPointer(&paint);
    drawCurrentVelocity(&paint);

}

void DashBoard::drawDashBoard(QPainter *paint)
{
    paint->save();

    QConicalGradient con(pointX,pointY,270);
    con.setColorAt(0,Qt::red);
    con.setColorAt(0.5,Qt::yellow);
    con.setColorAt(1,Qt::green);


    QPen pen;
    pen.setColor(QColor(80,80,80));//最外侧圆环
    paint->setBrush(QColor(80,80,80));
    QRectF rectangle(0, 0, formHeight, formWidth);
    paint->setPen(pen);
    paint->drawEllipse(rectangle);

    /***********************************************************/

    rectangle.setRect(OUTERMOST_WIIDTH,// 刻度彩色圆环
                      OUTERMOST_WIIDTH,
                      formHeight-(OUTERMOST_WIIDTH)*2,
                      formWidth-(OUTERMOST_WIIDTH)*2);//设定扇形所在的矩形，扇形画在矩形内部
    int startAngle = -45*16;//起始角度，角度可以为负值，如-30*16
    int spanAngle = 270*16;//覆盖的角度，绘制方向为逆时针方向
    pen.setColor(Qt::red);   // 刻度圆环
    paint->setBrush(con);
    paint->drawPie(rectangle, startAngle, spanAngle);
    /***********************************************************/
     rectangle.setRect(OUTERMOST_WIIDTH, OUTERMOST_WIIDTH,
                     formHeight-(OUTERMOST_WIIDTH)*2, formWidth-(OUTERMOST_WIIDTH)*2);//设定扇形所在的矩形，扇形画在矩形内部
    startAngle = -45 * 16;//起始角度，角度可以为负值，如-30*16
    spanAngle = (double)270/maxValue * (maxValue-value) * 16 ;//覆盖的角度，绘制方向为逆时针方向
    paint->setBrush(Qt::black);
    paint->drawPie(rectangle, startAngle, spanAngle);
    /***********************************************************/

    pen.setColor(QColor(80,80,80)); //倒数第三个圆
    paint->setBrush(QColor(80,80,80));
    rectangle.setRect(SCALE_WIIDTH+OUTERMOST_WIIDTH,
                      SCALE_WIIDTH+OUTERMOST_WIIDTH,
                      formHeight-(SCALE_WIIDTH+OUTERMOST_WIIDTH)*2,
                      formWidth-(SCALE_WIIDTH+OUTERMOST_WIIDTH)*2);
    paint->setPen(pen);
    paint->drawEllipse(rectangle);

    pen.setColor(QColor(255,123,106));//画中心圆
    paint->setBrush(QColor(255,123,106));
    rectangle.setRect(formWidth/2-CENTER_WIIDTH/2, formHeight/2-CENTER_WIIDTH/2,
                      CENTER_WIIDTH, CENTER_WIIDTH);
    paint->setPen(pen);
    paint->drawEllipse(rectangle);
    paint->restore();
}

void DashBoard::drawDivideRule(QPainter *paint)
{
    paint->save();
    QPen pen;
    pen.setColor(Qt::white);           // 设置画笔为黄色
    pen.setWidth(1);
    paint->setPen(pen);                // 设置画笔
    double averageAngle = (360.0-beginAngle*2) / (largeScaleNum * smallScaleNum);
    for(int i=0; i<=largeScaleNum * smallScaleNum; i++)
    {
        double angle = beginAngle+averageAngle*i;
        if(i % smallScaleNum == 0)
        {
            pen.setWidth(2);
            paint->setPen(pen);                // 设置画笔
            paint->drawLine(getCoordinates(formWidth/2-OUTERMOST_WIIDTH-1, angle),
                            getCoordinates(formWidth/2-(OUTERMOST_WIIDTH+SCALE_WIIDTH)+2, angle));// +2 ，偏移量
        }
        else
        {
            pen.setWidth(1);
            paint->setPen(pen);                // 设置画笔
            paint->drawLine(getCoordinates(formWidth/2-OUTERMOST_WIIDTH-1, angle),
                            getCoordinates(formWidth/2-(OUTERMOST_WIIDTH+SCALE_WIIDTH)+2+SCALE_WIIDTH/5, angle));// +2 ，偏移量
        }
    }
    paint->restore();
}

void DashBoard::drawNum(QPainter *paint)
{
    paint->save();
    QRectF rect(0, 0, 100, 100);
    paint->setPen(Qt::white);
    paint->setFont(QFont("Arial", 15));
    double averageAngle = (360.0-beginAngle*2) / (largeScaleNum * smallScaleNum);
    QPoint offset;
    offset.setX(0);
    offset.setY(0);
    for(int i=0; i<=largeScaleNum * smallScaleNum; i++)
    {
        double angle = beginAngle+averageAngle*i;
        QPoint point  = getCoordinates(formWidth/2-(OUTERMOST_WIIDTH+SCALE_WIIDTH)+2, angle);//起点坐标
        if(i % smallScaleNum == 0)
        {
            offset = getDrawNumOffset(frontW,frontH,angle);
            rect.setRect(point.x() + offset.x(), point.y() + offset.y(), frontW,frontH);
            paint->drawText(rect, Qt::AlignCenter,
                            QString("%1").arg( maxValue / largeScaleNum * (i/smallScaleNum)));
        }
    }
    paint->restore();
}

void DashBoard::drawPointer(QPainter *paint)
{
    paint->save();

    double angle = 45 + (double)(315-45)/maxValue * value;
    QPoint point = getCoordinates(formWidth/2-(OUTERMOST_WIIDTH+SCALE_WIIDTH), angle);

    double a =  angle*3.14159 / 180;//角度转弧度
    QPoint point1 ;
    point1.setX(pointX - TRIANGLE_WIDTH * cos(a));
    point1.setY(pointY - TRIANGLE_WIDTH * sin(a));

    QPoint point2;
    a = (90-angle)*3.14159 / 180;//角度转弧度
    point2.setX(pointX + TRIANGLE_WIDTH * sin(a));
    point2.setY(pointY + TRIANGLE_WIDTH * cos(a));

    QPolygonF polygon;
    polygon << point << point1 << point2 ;

    paint->setPen(QPen(QColor(255,123,106)));
    paint->setBrush(QColor(255,123,106));
    paint->drawPolygon(polygon, Qt::WindingFill);

    QPen pen;
    QRectF rectangle;
    pen.setColor(QColor(255,123,106));
    paint->setBrush(QColor(255,123,106));
    rectangle.setRect(formWidth/2-TRIANGLE_WIDTH-3,
                      formHeight/2-TRIANGLE_WIDTH-3,
                      (TRIANGLE_WIDTH+3)*2,
                      (TRIANGLE_WIDTH+3)*2);
    paint->setPen(pen);
    paint->drawEllipse(rectangle);

    pen.setColor(QColor(254,254,254));//画中心圆
    paint->setBrush(QColor(254,254,254));
    rectangle.setRect(formWidth/2-WHITE_WIDTH/2, formHeight/2-WHITE_WIDTH/2,
                      WHITE_WIDTH, WHITE_WIDTH);
    paint->setPen(pen);
    paint->drawEllipse(rectangle);
    paint->restore();
}

void DashBoard::drawCurrentVelocity(QPainter *paint)
{
    paint->save();
    QRectF rect;
    int FH = 80, FW = 60;
    paint->setPen(Qt::white);
    paint->setFont(QFont("Arial", 20));
    rect.setRect(pointX-FW/2, pointY + pointY*2/7, FW, FH);
    paint->drawText(rect, Qt::AlignCenter, QString("km/h"));

    FH = 40, FW = 60;

    QFont font;
    font.setBold(false);
    font.setPointSize(25);
    font.setFamily("Microsoft YaHei");
    font.setLetterSpacing(QFont::PercentageSpacing,0);
    paint->setFont(font);
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(2);
    paint->setPen(pen);
    rect.setRect(pointX-FW/2, pointY - FH/2, FW, FH);
    paint->drawText(rect, Qt::AlignCenter, QString("%1").arg(value));
    paint->restore();
}

QPoint DashBoard::getCoordinates(int r, double angle)
{
    angle = angle*3.14159 / 180;//角度转弧度
    QPoint point;

    point.setX(formWidth/2 - (int)((double)r * sin(angle)));
    point.setY(formHeight/2 + (int)((double)r * cos(angle)));

    return point;
}


QPoint DashBoard::getDrawNumOffset(int frontwidth, int frontHeight, double angle)
{
    QPoint point;
    if(angle>=45 && angle < 90 )
    {
        double v = (double)frontHeight/35;
        point.setX(0);
        point.setY(-(90-angle) * v );
    }
    else if(angle == 90)
    {
        point.setX(0);
        point.setY(-frontHeight/2 );
    }
    else if(angle >90 && angle <180){
        double v = ((double)frontHeight/4)/45;
        point.setX(-(angle - 90) * v + 4);
        point.setY(-(180-angle) * v + 4);
    }else if(angle == 180)
    {
        point.setY(5);
        point.setX(-frontwidth/2);
    }else if(angle > 180 && angle < 225)
    {
        double v = (double)frontHeight/90;
        point.setX(-(frontwidth)+6);
        if(angle >= 225)
            point.setY(-(angle - 225) * v);
        else
            point.setY(0);
    }
    else if(angle >=225 && angle <=315)
    {
        double v = (double)frontHeight/90;
        point.setX(-(frontwidth + 5));
        if(angle >= 225)
            point.setY(-(angle - 225) * v);
        else
            point.setY(0);
    }
    return point;
}

