
#include "VideoRenderer/videorenderer.h"
#include "Logger/logger.h"


int width = 1920;
int height = 1080;
GL_Image::GL_Image(QWidget* parent):
    QOpenGLWidget(parent)
{
    imageData_ = nullptr;
    dragFlag_ = false;
    scaleVal_ = 1.0;
}

// 设置待显示的数据源和尺寸
void GL_Image::setImageData(uchar* imageSrc, uint width, uint height)
{
    imageData_ = imageSrc;
    imageSize_.setWidth(width);
    imageSize_.setHeight(height);
}

void GL_Image::AVFrameSlot(AVFrame* frame)
{
    frameList.push_back(frame);
    // LOG_DEBUG() << "frame list size: " << frameList.size();
    // LOG_DEBUG() << "w: " << frame->width << " h: " << frame->height;
    // return;
    // m_frame = frame;
    imageSize_.setWidth(frame->width);
    imageSize_.setHeight(frame->height);
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


void p010ToRGBA(uint16_t y, uint16_t u, uint16_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
    // Convert YUV to RGB
    int Y = y - 64;
    int U = u - 512;
    int V = v - 512;

    int R = Y + (1.402 * V);
    int G = Y - (0.344136 * U) - (0.714136 * V);
    int B = Y + (1.772 * U);

    // Clip RGB values to 0-255
    r = std::clamp(R, 0, 255);
    g = std::clamp(G, 0, 255);
    b = std::clamp(B, 0, 255);
}

// Convert P010LE to RGBA
void convertP010ToRGBA(uint8_t* p010Data, uint8_t* rgbaData) {
    int stride = width * 2; // P010 uses 2 bytes per pixel

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Calculate the pixel position
            int p010Index = y * stride + x * 2;

            // Extract YUV components (P010LE has 10-bit YUV, packed in 16-bit words)
            uint16_t y = (p010Data[p010Index] << 2) | (p010Data[p010Index + 1] >> 6);
            uint16_t u = ((p010Data[p010Index + 1] & 0x3F) << 4) | (p010Data[p010Index + 2] >> 4);
            uint16_t v = ((p010Data[p010Index + 2] & 0x0F) << 6) | (p010Data[p010Index + 3] >> 2);

            // Convert YUV to RGB
            uint8_t r, g, b;
            p010ToRGBA(y, u, v, r, g, b);

            // Store RGBA values (each pixel is 4 bytes in RGBA format)
            int rgbaIndex = y * width * 4 + x * 4;
            rgbaData[rgbaIndex] = r;
            rgbaData[rgbaIndex + 1] = g;
            rgbaData[rgbaIndex + 2] = b;
            rgbaData[rgbaIndex + 3] = 255; // Alpha channel (opaque)
        }
    }
}

// 窗口绘制函数
void GL_Image::paintGL()
{
    if (frameList.size() <= 0) {
        return;
    }
    static int idx = 0;
    m_frame = frameList[idx++];
    if (idx >= frameList.size()) {
        idx = 0;
    }
    static bool initTextureFlag = false;
    // 设置背景颜色
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(m_frame == nullptr){
        return;
    }

    glBindTexture(GL_TEXTURE_2D, textureId_);

    uint8_t *rgbaData = new uint8_t[width * height * 2 + (width / 2) * (height / 2) * 4];
    uint8_t *d = m_frame->data;
    convertP010ToRGBA(m_frame->data, rgbaData);

    // ConvertToRGB(rgbaData, imageSize_, m_frame);

    // SwsContext *swsCtx = sws_getContext(
    //     m_frame->width, m_frame->height, (const AVPixelFormat)m_frame->format,
    //     m_frame->width, m_frame->height, AV_PIX_FMT_RGBA,
    //     SWS_BILINEAR, nullptr, nullptr, nullptr);
    // uint8_t *rgbaData = (uint8_t *)av_malloc(width * height * 2 + (width / 2) * (height / 2) * 4);
    // sws_scale(swsCtx, m_frame->data, m_frame->linesize, 0, m_frame->height, &rgbaData, m_frame->linesize);

    if(!initTextureFlag)
    {
        // 生成纹理
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageSize_.width(), imageSize_.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
        glTexImage2D(GL_TEXTURE_2D, 0, 
                    GL_RGBA, 
                    imageSize_.width(), 
                    imageSize_.height(), 
                    0, 
                    GL_RGBA, 
                    GL_UNSIGNED_BYTE, 
                    rgbaData);

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
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        imageSize_.width(), imageSize_.height(),
                        GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);

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
    // 交换前后缓冲区
    // swapBuffers();
}

void GL_Image::resizeGL(int w, int h)
{
    // 传入的w，h时widget控件的尺寸
    Ortho2DSize_.setWidth(w);
    Ortho2DSize_.setHeight(h);
    glViewport(0,0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Ortho2DSize_.width(), Ortho2DSize_.height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
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
