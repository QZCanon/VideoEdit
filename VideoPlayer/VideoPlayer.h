#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QThread>
#include <sys/time.h>

#include "Logger/logger.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

class VideoPlayer : public QThread {
    Q_OBJECT

public:
    VideoPlayer(const QString &filename, QLabel *label)
        : label(label), stop(false), formatContext(nullptr), codecContext(nullptr), swsContext(nullptr)
    {
        avformat_network_init();
        if (avformat_open_input(&formatContext, filename.toStdString().c_str(), nullptr, nullptr) < 0) {
            qWarning("Could not open video file.");
            return;
        }
        if (avformat_find_stream_info(formatContext, nullptr) < 0) {
            qWarning("Could not find stream information.");
            return;
        }

        // Find the video stream by manually checking stream types
        videoStreamIndex = -1;
        for (int i = 0; i < formatContext->nb_streams; ++i) {
            if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex < 0) {
            qWarning("Could not find video stream.");
            return;
        }

        codecContext = avcodec_alloc_context3(nullptr);
        if (!codecContext) {
            qWarning("Could not allocate codec context.");
            return;
        }

        if (avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar) < 0) {
            qWarning("Could not copy codec parameters to codec context.");
            return;
        }

        if (avcodec_open2(codecContext, avcodec_find_decoder(codecContext->codec_id), nullptr) < 0) {
            qWarning("Could not open codec.");
            return;
        }
        LOG_DEBUG() << "videl player init is success";
    }

    ~VideoPlayer() {
        stopPlayback();
        if (swsContext) {
            sws_freeContext(swsContext);
        }
        if (codecContext) {
            avcodec_free_context(&codecContext);
        }
        if (formatContext) {
            avformat_close_input(&formatContext);
        }
        avformat_network_deinit();
    }

    uint8_t *rgbData;
    int rgbLinesize[4];
    QImage image;
    bool imageInitialized = false;

    void run() override {
        AVFrame *frame = av_frame_alloc();
        AVPacket packet;

        LOG_DEBUG() << "run";
        while (1) {
            if (stop) {
                LOG_DEBUG() << "exit thread";
                break;
            }
            if (av_read_frame(formatContext, &packet) < 0) {
                LOG_DEBUG() << "read frame fail";
                break;
            }
            if (packet.stream_index == videoStreamIndex) {
                if (avcodec_send_packet(codecContext, &packet) == 0) {
                    if (avcodec_receive_frame(codecContext, frame) == 0) {
                        LOG_DEBUG() << "---------------------------";
                        double t1 = (double)clock() / 1000;
                        // Convert frame to QImage
                        if (!swsContext) {
                            swsContext = sws_getCachedContext(
                                nullptr, codecContext->width, codecContext->height, codecContext->pix_fmt,
                                codecContext->width, codecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr
                            );
                        }
                        double t2 = (double)clock() / 1000;
                        uint8_t *data[4];
                        int linesize[4];
                        av_image_alloc(data, linesize, codecContext->width, codecContext->height, AV_PIX_FMT_RGB24, 1);
                        double t3 = (double)clock() / 1000;
                        sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, data, linesize);
                        double t4 = (double)clock() / 1000;
                        QImage image(data[0], codecContext->width, codecContext->height, QImage::Format_RGB888);
                        QPixmap pixmap = QPixmap::fromImage(image).scaled(label->size(), Qt::KeepAspectRatio);
                        QMetaObject::invokeMethod(label, "setPixmap", Qt::QueuedConnection, Q_ARG(QPixmap, pixmap));
                        double t5 = (double)clock() / 1000;
                        LOG_DEBUG() << "sws_getCachedContext time: " << t2 - t1 << "(ms)";
                        LOG_DEBUG() << "av_image_alloc time:       " << t3 - t2 << "(ms)";
                        LOG_DEBUG() << "sws_scale time:            " << t4 - t3 << "(ms)";
                        LOG_DEBUG() << "draw pic time:             " << t5 - t4 << "(ms)";
                        // av_freep(&data[0]);
                    }
                }
            }
            av_packet_unref(&packet);
            msleep(40); // Adjust timing based on video frame rate
        }
        LOG_DEBUG() << "frame free";
        av_frame_free(&frame);
    }

    void stopPlayback() { stop = true; }

private:
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    SwsContext *swsContext;
    int videoStreamIndex;
    QLabel *label;
    bool stop;
};

#endif // VIDEOPLAYER_H
