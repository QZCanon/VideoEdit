#ifndef VIDEO_HW_DECODER_H
#define VIDEO_HW_DECODER_H

#include <QObject>

#include <thread>

#include "core/ring_buffer.hpp"
#include "task_runner/task_runner.hpp"

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


class HwDecoder : public QObject
{
    Q_OBJECT
public:
    explicit HwDecoder(QObject *parent = nullptr);
    ~HwDecoder();
    int Init();
    void Start();
    auto GetFileFPS() const { return m_fps; }
    AVFrame* GetFrame()     { return m_frameList.FrontAndPop(); }
    bool BufferIsEmpty()    { return m_frameList.Empty(); }
    auto GetCreateTime()    { return m_createTime; }

signals:

private:
    int DecodeWrite(AVCodecContext *avctx, AVPacket *packet);
    int HwDecoderInit(AVCodecContext *ctx, const AVHWDeviceType type);
    void DoWork();

private:
    AVFormatContext *input_ctx = NULL;
    int video_stream, video_ret;
    AVStream *video = NULL;
    AVCodecContext *decoder_ctx = NULL;
    AVCodec *decoder = NULL;
    AVPacket packet;
    enum AVHWDeviceType type;
    // int i;
    AVBufferRef *hw_device_ctx = NULL;
    std::thread th;
    uint8_t m_fps = 0;
    uint64_t m_createTime = -1;

    bool isExitDecode = false;

    Task task;
    TaskRunner *runner{nullptr};

    RingBuffer<AVFrame*, 3> m_frameList;

#if defined(Q_OS_MAC)
    std::string hwdevice  = "videotoolbox";
    // std::string inputName = "/Users/qinzhou/workspace/test/input_file.mp4";
    std::string inputName = "/Users/qinzhou/workspace/test/DJI_20240820194031_0041_D.MP4";
#elif defined(Q_OS_WIN)
    std::string hwdevice  = "dxva2";
    std::string inputName = "F:/大疆/拍摄/0830/DJI_20240830191244_0060_D.MP4";
#endif

};

#endif // VIDEO_HW_DECODER_H