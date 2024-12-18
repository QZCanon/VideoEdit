
#include "VideoRenderer/videorenderer.h"
#include "Logger/logger.h"
#include "core/utils.h"
#include <sys/time.h>>

GL_Image::GL_Image(QWidget* parent):
    QOpenGLWidget(parent)
{
    imageData_ = nullptr;
    dragFlag_ = false;
    scaleVal_ = 1.0;
}

GL_Image::~GL_Image()
{

}


void GL_Image::initializeGL()
{
    // 生成一个纹理ID
    glGenTextures(1, &textureId_);
    // 绑定该纹理ID到二维纹理上
    glBindTexture(GL_TEXTURE_2D, textureId_);
    // 用线性插值实现图像缩放
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void GL_Image::SetFrame(Canon::VideoFrame* frame)
{
    if (!m_painting) {
        m_painting = true;
        m_frame = frame;
    }
}

// 窗口绘制函数
void GL_Image::paintGL()
{
    // 设置背景颜色
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(m_frame == nullptr) {
        m_painting = false;
        // LOG_DEBUG() << "frame is null";
        return;
    }

    // uint8_t *rgbaData = (uint8_t*)malloc(m_frame->width * m_frame->height * 4);
    // convert_hardware_yuv_to_rgba(m_frame->data[0],
    //                              m_frame->data[1],
    //                              rgbaData,
    //                              m_frame->width,
    //                              m_frame->height,
    //                              (AVPixelFormat)m_frame->format);

    QSize imageSize_;            //图片尺寸
    imageSize_.setWidth(m_frame->width);
    imageSize_.setHeight(m_frame->height);

    if (!m_frame && m_frame->frame == nullptr) {
        m_painting = false;
        LOG_DEBUG() << "rgbaData is null";
        return;
    }

    glBindTexture(GL_TEXTURE_2D, textureId_);
    static bool initTextureFlag = false;
    if(!initTextureFlag)
    {
        // 生成纹理
        glTexImage2D(GL_TEXTURE_2D, 0,
                    GL_RGBA,
                    imageSize_.width(),
                    imageSize_.height(),
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    m_frame->frame);

        // 初始化顶点坐标（居中显示）
        int x_offset = 0;
        int y_offset = 0;
        if(imageSize_.width()<Ortho2DSize_.width() && imageSize_.height()<Ortho2DSize_.height())
        {
            // 当图片宽、高都比窗口尺寸小时，只需计算贴图时在水平和垂直方向上的偏移即可
            x_offset = (Ortho2DSize_.width()-imageSize_.width()) / 2;
            y_offset = (Ortho2DSize_.height()-imageSize_.height()) / 2;
        }
        else
        {
            // 当图片宽或高比窗口尺寸大时，需再要先对图片做等比例缩放，计算贴图时在水平和垂直方向上的偏移
            float w_rate = float(Ortho2DSize_.width()) / imageSize_.width();
            float h_rate = float(Ortho2DSize_.height()) / imageSize_.height();
            if(w_rate < h_rate)
            {
                x_offset = 0;
                y_offset = (Ortho2DSize_.height()-imageSize_.height()*w_rate) / 2;
            }
            else
            {
                x_offset = (Ortho2DSize_.width()-imageSize_.width()*h_rate) / 2;
                y_offset = 0;
            }
        }

        adaptImageSize_.setWidth(Ortho2DSize_.width()-2*x_offset);
        adaptImageSize_.setHeight(Ortho2DSize_.height()-2*y_offset);

        // 顶点坐标保存的是相对于窗口的位置
        vertexPos_[Left_Bottom_X] = x_offset;
        vertexPos_[Left_Bottom_Y] = y_offset;
        vertexPos_[Right_Bottom_X] = Ortho2DSize_.width() - x_offset;
        vertexPos_[Right_Bottom_Y] = y_offset;
        vertexPos_[Right_Top_X] = Ortho2DSize_.width() - x_offset;
        vertexPos_[Right_Top_Y] = Ortho2DSize_.height() - y_offset;
        vertexPos_[Left_Top_X] = x_offset;
        vertexPos_[Left_Top_Y] = Ortho2DSize_.height() - y_offset;

        // 纹理坐标点保存的是相对于图片的位置
        texturePos_[Left_Bottom_X] = 0.0f;
        texturePos_[Left_Bottom_Y] = 0.0f;
        texturePos_[Right_Bottom_X] = 1.0f;
        texturePos_[Right_Bottom_Y] = 0.0f;
        texturePos_[Right_Top_X] = 1.0f;
        texturePos_[Right_Top_Y] = 1.0f;
        texturePos_[Left_Top_X] = 0.0f;
        texturePos_[Left_Top_Y] = 1.0f;

        initTextureFlag = true;
    }
    else
    {
        // 第一次显示用glTexImage2D方式显示，后面用glTexSubImage2D动态修改纹理数据的方式显示
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        imageSize_.width(),
                        imageSize_.height(),
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        m_frame->frame);

    }
    // free(rgbaData);
    if (m_frame) {
        if (m_frame->frame) {
            delete m_frame->frame;
            m_frame->frame = nullptr;
        }
        delete m_frame;
        m_frame = nullptr;
    }

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glTexCoord2d(texturePos_[Left_Bottom_X], texturePos_[Left_Bottom_Y]);
    glVertex2d(vertexPos_[Left_Bottom_X], vertexPos_[Left_Bottom_Y]);
    glTexCoord2d(texturePos_[Left_Top_X], texturePos_[Left_Top_Y]);
    glVertex2d(vertexPos_[Left_Top_X], vertexPos_[Left_Top_Y]);
    glTexCoord2d(texturePos_[Right_Top_X], texturePos_[Right_Top_Y]);
    glVertex2d(vertexPos_[Right_Top_X], vertexPos_[Right_Top_Y]);
    glTexCoord2d(texturePos_[Right_Bottom_X], texturePos_[Right_Bottom_Y]);
    glVertex2d(vertexPos_[Right_Bottom_X], vertexPos_[Right_Bottom_Y]);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    m_painting = false;
}

void GL_Image::resizeGL(int w, int h)
{
    LOG_DEBUG() << w << " * " << h;
    // 传入的w，h时widget控件的尺寸
    Ortho2DSize_.setWidth(w);
    Ortho2DSize_.setHeight(h);
    glViewport(0,0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Ortho2DSize_.width(), Ortho2DSize_.height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    paintGL();
}

// 鼠标滚轮实现图片倍率缩放,缩放间隔为0.1，调整绘制位置的偏移，调用paintGL重绘
void GL_Image::wheelEvent(QWheelEvent* e)
{
    // if(e->delta() > 0)
    // {
    //     scaleVal_ += 0.1;
    //     scaleVal_ = scaleVal_>3? 3:scaleVal_;
    // }
    // else
    // {
    //     scaleVal_ -= 0.1;
    //     scaleVal_ = scaleVal_<0.1? 0.1:scaleVal_;
    // }

    // uint16_t showImgWidth = adaptImageSize_.width() * scaleVal_;
    // uint16_t showImgHeight = adaptImageSize_.height() * scaleVal_;

    // int xoffset = (Ortho2DSize_.width() - showImgWidth)/2;
    // int yoffset = (Ortho2DSize_.height() - showImgHeight)/2;

    // vertexPos_[Left_Bottom_X] = xoffset;
    // vertexPos_[Left_Bottom_Y] = yoffset;
    // vertexPos_[Right_Bottom_X] = xoffset + showImgWidth;
    // vertexPos_[Right_Bottom_Y] = yoffset;
    // vertexPos_[Right_Top_X] = xoffset + showImgWidth;
    // vertexPos_[Right_Top_Y] = yoffset + showImgHeight;
    // vertexPos_[Left_Top_X] = xoffset;
    // vertexPos_[Left_Top_Y] = yoffset + showImgHeight;

    // paintGL();
}

// 实现鼠标拖拽图片，鼠标在拖拽过程中会反复调用此函数，因此一个连续的拖拽过程可以
// 分解为多次移动的过程，每次移动都是在上一个位置的基础上进行一次位置调节
void GL_Image::mouseMoveEvent(QMouseEvent* e)
{
    if (dragFlag_)
    {
        int scaledMoveX = e->x()-dragPos_.x();
        int scaledMoveY = e->y()-dragPos_.y();

        vertexPos_[Left_Bottom_X] += scaledMoveX;
        vertexPos_[Left_Bottom_Y] += scaledMoveY;
        vertexPos_[Left_Top_X] += scaledMoveX;
        vertexPos_[Left_Top_Y] += scaledMoveY;
        vertexPos_[Right_Top_X] += scaledMoveX;
        vertexPos_[Right_Top_Y] += scaledMoveY;
        vertexPos_[Right_Bottom_X] += scaledMoveX;
        vertexPos_[Right_Bottom_Y] += scaledMoveY;

        dragPos_.setX(e->x());
        dragPos_.setY(e->y());
        paintGL();
    }
}

void GL_Image::mousePressEvent(QMouseEvent* e)
{
    if(scaleVal_ > 0)
    {
        dragFlag_ = true;
        dragPos_.setX(e->x());
        dragPos_.setY(e->y());
    }
}

void GL_Image::mouseReleaseEvent(QMouseEvent* e)
{
    dragFlag_ = false;
}

// 双击实现原比例显示，缩放倍率设置为1.0
void GL_Image::mouseDoubleClickEvent(QMouseEvent* e)
{
    scaleVal_ = 1.0;

    uint16_t showImgWidth = adaptImageSize_.width() * scaleVal_;
    uint16_t showImgHeight = adaptImageSize_.height() * scaleVal_;

    int xoffset = (Ortho2DSize_.width() - showImgWidth)/2;
    int yoffset = (Ortho2DSize_.height() - showImgHeight)/2;

    vertexPos_[Left_Bottom_X] = xoffset;
    vertexPos_[Left_Bottom_Y] = yoffset;
    vertexPos_[Right_Bottom_X] = xoffset + showImgWidth;
    vertexPos_[Right_Bottom_Y] = yoffset;
    vertexPos_[Right_Top_X] = xoffset + showImgWidth;
    vertexPos_[Right_Top_Y] = yoffset + showImgHeight;
    vertexPos_[Left_Top_X] = xoffset;
    vertexPos_[Left_Top_Y] = yoffset + showImgHeight;

    paintGL();
}
