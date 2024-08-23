#ifndef DECODER_H
#define DECODER_H

#include <QObject>

#include <thread>

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


class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(QObject *parent = nullptr);
    ~Decoder();
    int Init();
    void Start();
    int DoWork();

signals:
    void DecoderSendAVFrame(AVFrame*);
    void DecoderIsFinish();

private:
    int DecodeWrite(AVCodecContext *avctx, AVPacket *packet);
    int HwDecoderInit(AVCodecContext *ctx, const AVHWDeviceType type);

private:
    AVFormatContext *input_ctx = NULL;
    int video_stream, ret;
    AVStream *video = NULL;
    AVCodecContext *decoder_ctx = NULL;
    AVCodec *decoder = NULL;
    AVPacket packet;
    enum AVHWDeviceType type;
    // int i;
    AVBufferRef *hw_device_ctx = NULL;
    std::thread th;

    bool isExitDecode = false;

#if defined(Q_OS_MAC)
    std::string hwdevice  = "videotoolbox";
    std::string inputName = "/Users/qinzhou/workspace/test/DJI_20240820194031_0041_D.MP4";
#elif defined(Q_OS_WIN)
    std::string hwdevice  = "dxva2";
    std::string inputName = "F:/test.mp4";
#endif

};

#endif // DECODER_H
