#ifndef VIDEO_HW_DECODER_H
#define VIDEO_HW_DECODER_H

#include <QObject>

#include <thread>

#include "core/ring_buffer.hpp"
#include "core/types.h"
#include "core/atomic_vector.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

enum class DecodeType {
    INIT,
    KEY_FRAME,
    ALL_FRAME,
    UNKNOWN,
};


class HwDecoder : public QObject
{
    Q_OBJECT
public:
    // HwDecoder() {}
    explicit HwDecoder(const std::string& inputName, QObject *parent = nullptr): QObject{parent}
    {
        Init(inputName);
    }
    virtual ~HwDecoder();

    // 开始解码
    int Start();

    // 获取解码帧
    Canon::VideoFrame* GetFrame()
    {
        return m_frameList.FrontAndPop();
    }

    uint64_t GetPaintFrameDuration() { return m_frameList.ReadPos()->duration; }

    void BufferWait() { m_frameList.Wait(); }

    // 判断解码缓存队列是否为空
    bool BufferIsEmpty()    { return m_frameList.Empty(); }

    // 获取视频录制时间
    auto GetCreateTime()    { return m_createTime; }

    // 获取视频帧率
    auto GetFileFPS() const { return m_fps; }

    // 重头开始播放
    int Restart();

    // 从某关键帧开始播放
    void StartFromKeyFrameAsync(const Canon::VideoKeyFrame keyFrame);

private:
    // 初始化资源
    int Init(const std::string& inputName);

    // 解码核心函数
    int Decoder(AVCodecContext *avctx, AVPacket *packet);

    // 创建和绑定硬件
    int HwDecoderInit(AVCodecContext *ctx, const AVHWDeviceType type);

    // 解码线程执行函数
    void DoWork();

    // 状态切换
    void StateSwitching();

    // 设置关键帧
    void SetKeyFrame(const Canon::VideoKeyFrame keyFrame);

private:
    // ffmpeg资源相关
    AVFormatContext* input_ctx = NULL;
    int              video_stream;
    int              video_ret;
    AVCodecContext*  decoder_ctx = NULL;
    AVPacket         packet;
    AVBufferRef*     hw_device_ctx = NULL;

    double            m_fps = 0;
    uint64_t          m_createTime = -1;

    std::atomic<bool>       m_isDecoding{false};
    std::thread             m_decodeThread;
    bool                    m_isExitDecode{false};
    std::atomic<DecodeType> m_decodeType{DecodeType::UNKNOWN};
    std::condition_variable m_decodeCond;
    std::mutex              m_mutex;
    double                  m_timeInterval{0.0}; // 当前帧的持续时间

    AtomicVector<Canon::VideoKeyFrame> m_keyFrameList;
    RingBuffer<Canon::VideoFrame*, 5>            m_frameList;

#if defined(Q_OS_MAC)
    std::string hwdevice  = "videotoolbox";
#elif defined(Q_OS_WIN)
    std::string hwdevice  = "dxva2";
#endif

};

#endif // VIDEO_HW_DECODER_H
