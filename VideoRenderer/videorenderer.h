#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <QWheelEvent>
#include <QOpenGLWidget>
#include <QTimer>
#include "core/types.h"

#include <thread>

class GL_Image : public QOpenGLWidget
{
    Q_OBJECT

public:
    enum
    {
        Left_Bottom_X,
        Left_Bottom_Y,
        Right_Bottom_X,
        Right_Bottom_Y,
        Right_Top_X,
        Right_Top_Y,
        Left_Top_X,
        Left_Top_Y,
        Pos_Max
    };

    GL_Image(QWidget* parent = nullptr);
    ~GL_Image();

    void SetFrame(Canon::VideoFrame* frame);

    bool BePainting() { return m_painting; }
private:

public slots:

protected:
    // 重写QGLWidget类的接口
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    // 鼠标事件
    void wheelEvent(QWheelEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);

private:
    uchar* imageData_;           //纹理显示的数据源

    QSize adaptImageSize_;       //适配尺寸
    QSize Ortho2DSize_;          //窗口尺寸
    GLuint textureId_;           //纹理对象ID
    int vertexPos_[Pos_Max];     //窗口坐标
    float texturePos_[Pos_Max];  //纹理坐标
    bool dragFlag_;              //鼠标拖拽状态
    QPoint dragPos_;             //鼠标拖拽位置
    float scaleVal_;             //缩放倍率

    Canon::VideoFrame* m_frame;
    bool m_painting{false};
};

#endif // VIDEORENDERER_H
