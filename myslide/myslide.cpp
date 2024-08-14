#include "myslide.h"
#include "ui_myslide.h"

MySlide::MySlide(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MySlide)
{
    ui->setupUi(this);
    this->setMouseTracking(true);      //设置为不按下鼠标键触发moveEvent
}

MySlide::~MySlide()
{
    delete ui;
}

void MySlide::setValue(const int value)
{
    if(value >= 0 && value <= 100)
        this->progressValue = value;
    repaint();
}

void MySlide::setButtonHeight(int h)
{
    if(h >= 0)
        this->buttonHeight = h;
    repaint();
}

void MySlide::setButtonAutoHide(bool isAutoHide)
{
    buttonAutoHide = isAutoHide;
    if(!isAutoHide)
        buttonIsVisibe = true;
    else
        buttonIsVisibe = false;
    repaint();
}

void MySlide::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p_re = event->pos();
    int sub = qAbs(p_re.x() - pressPoint.x());


    if(p_re.rx() >= btnX
            && p_re.rx() <= btnX+buttonHeight
            && p_re.ry() >= btnY
            && p_re.ry() <= btnY+buttonHeight)
    {
        this->setCursor(Qt::PointingHandCursor);
        if(buttonAutoHide)
            buttonIsVisibe = true;
    }
    else
    {
        this->setCursor(Qt::ArrowCursor);      //范围之外变回原来形状
        if(buttonAutoHide && !isPressBtn)
            buttonIsVisibe = false;
    }

    repaint();

    if(!isPressBtn)
        return;
    //    qDebug() << "sub: " << sub;
    static int srcValue = 0;
    int t = 0;
    if(saveVal)
    {
        saveVal = false;
        srcValue = progressValue;

    }
    if(p_re.x() - pressPoint.x() >= 0)//鼠标右滑
    {
        t = (float)100/progressSumPix * sub;
        progressValue = srcValue+t;
        if(progressValue >= 100)
            progressValue = 100;
    }else//鼠标左滑
    {
        t = (float)100/progressSumPix * sub;
        progressValue = srcValue-t;
        if(progressValue <= 0)
            progressValue = 0;
    }



    repaint();
}

void MySlide::mousePressEvent(QMouseEvent *event)
{
    pressPoint = event->pos();
    if(pressPoint.y() >= currentY && pressPoint.y() <= (currentY+4)
            &&( pressPoint.x()<=btnX || pressPoint.x()>= btnX+buttonHeight))
    {
        progressValue =  (float)100/progressSumPix * (pressPoint.x() - buttonHeight/2)+ 0.5;
    }
    if(pressPoint.x() >= btnX && pressPoint.x() <= btnX+buttonHeight &&
            pressPoint.y()>=btnY && pressPoint.y()<=btnY+buttonHeight)
    {
        //        qDebug() << "按钮被点击";
        isPressBtn = true;
        saveVal = true;
    }
    repaint();
}

void MySlide::mouseReleaseEvent(QMouseEvent *)
{
    isPressBtn = false;
}

void MySlide::paintEvent(QPaintEvent *)
{
    if(progressValue < 0 || progressValue>100)
        return;
    // 创建QPainter一个对象
    int w = this->width()-1;
    int h = this->height()-2;
    currentY = h/2 - lineHeight/2;
    int PSum = w - buttonHeight;//进度总长度，（减去按钮宽度）
    progressSumPix = PSum;
    int currentProgress = (int)((float)PSum/100*progressValue);

           QPainter painter(this) ;//背景
           painter.setBrush(Qt::green);//画刷QBrushQColor(0x00,0xFF,0x00)
           painter.setPen(QPen(QColor(232,60,60),1,Qt::SolidLine));//画笔
           // painter.drawRoundRect(0, 0,
           //                       w,h,
           //                       0,0); // TODO

    QPainter painter2(this) ;//灰色区域
    // painter2.setRenderHint(QPainter::Qt4CompatiblePainting);  // 反锯齿;
    painter2.setRenderHint(QPainter::Antialiasing, true);
    painter2.setBrush(QColor(194,194,196));//画刷
    painter2.setPen(QPen(QColor(194,194,196),1,Qt::SolidLine));//画笔
    //    QRectF rectangle3((float)PSum/100*progressValue+buttonHeight/2, currentY, PSum-currentProgress, lineHeight);
    QRectF rectangle3(buttonHeight/2, currentY, PSum, lineHeight);
    painter2.drawRoundedRect(rectangle3, 1.0, 1.0);

    QPainter painter1(this) ;//红色区域
    // painter1.setRenderHint(QPainter::Qt4CompatiblePainting);  // 反锯齿;
    painter1.setRenderHint(QPainter::Antialiasing, true);
    painter1.setBrush(QColor(232,60,60));//画刷QBrushQColor(0x00,0xFF,0x00)
    painter1.setPen(QPen(QColor(232,60,60),1,Qt::SolidLine));//画笔
    QRectF rectangle2(buttonHeight/2, currentY, currentProgress, lineHeight);
    painter1.drawRoundedRect(rectangle2, 1.0, 1.0);


    //    {
    //        QPainterPath painterPath;
    //        painterPath.addRoundedRect(rectangle3, 1.0, 1.0);
    //        painter2.drawPath(painterPath);
    //    }

    btnX = currentProgress;//保存button位置信息
    btnY = h/2-buttonHeight/2;
    if(!buttonIsVisibe)//不可见
        return;

    QPainter painter3(this) ;//画圆形button  221,221,222
    QRectF rectangle1(currentProgress, h/2-buttonHeight/2, buttonHeight, buttonHeight);
    //QPainter painter(this);
    painter3.setPen(QPen(QColor(198,198,198),1,Qt::SolidLine));//画笔
    painter3.setBrush(Qt::white);//画刷
    painter3.drawEllipse(rectangle1);

    QPainter painter4(this) ;//画圆形button内的红点
    painter4.setPen(QColor(232,60,60));
    painter4.setBrush(QColor(232,60,60));
    painter4.drawEllipse(btnX + buttonHeight/2-2,currentY,4,4);
    //Sleep(20);
    emit currentProgressValue(progressValue);

}

