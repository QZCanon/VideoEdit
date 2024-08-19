#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger/logger.h"

#include <QFileDialog>
#include <QTime>
#include "decoder/decoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include <stdio.h>
#include <stdlib.h>
#include <cstdio>

#include <time.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    test();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::test()
{
    const char *filename = "F:/test.mp4"; // 输入视频文件名
    AVFormatContext *pFormatCtx = NULL;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *dec_ctx = NULL;
    // AVCodec *codec = NULL;
    AVPacket packet;
    AVFrame *frame, *rgb_frame;
    int ret, got_picture;
    int y_size;
    FILE *yuv_file;

    // Open video file
    if ((ret = avformat_open_input(&pFormatCtx, filename, NULL, NULL)) < 0) {
        fprintf(stderr, "Could not open source file '%s'\n", filename);
        exit(1);
    }

    // Retrieve stream information
    if ((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information\n");
        exit(1);
    }

    // Find the first video stream
    int videoStream = -1;
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) {
        fprintf(stderr, "Could not find a video stream in the input file\n");
        exit(1);
    }

    // Get a pointer to the codec parameters for the video stream
    codecpar = pFormatCtx->streams[videoStream]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (codec == NULL) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    // Create a decoder context
    dec_ctx = avcodec_alloc_context3(codec);
    if ((ret = avcodec_parameters_to_context(dec_ctx, codecpar)) < 0) {
        fprintf(stderr, "Failed to copy codec parameters to decoder context\n");
        exit(1);
    }

    // 设置硬件加速解码器
    AVDictionary *options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "hwaccel", "cuda", 0); // 使用CUDA硬件加速

    // Open codec
    if ((ret = avcodec_open2(dec_ctx, codec, &options)) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    // Allocate frame
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        exit(1);
    }

    // Allocate an AVFrame structure
    rgb_frame = av_frame_alloc();
    if (!rgb_frame) {
        fprintf(stderr, "Could not allocate RGB video frame\n");
        exit(1);
    }

    // Determine required buffer size and allocate buffer
    y_size = dec_ctx->width * dec_ctx->height;
    uint8_t *out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(dec_ctx->pix_fmt, dec_ctx->width, dec_ctx->height, 1));

    // Set up the destination frame's data
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, out_buffer, dec_ctx->pix_fmt, dec_ctx->width, dec_ctx->height, 1);

    // Set scaling options
    SwsContext *sws_ctx = sws_getContext(dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                                         dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                                         SWS_BILINEAR, NULL, NULL, NULL);

    // Read frames from the file
    int frame_count = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            ret = avcodec_send_packet(dec_ctx, &packet);
            if (ret < 0) {
                fprintf(stderr, "Error submitting a packet for decoding\n");
                exit(1);
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit(1);
                }
                // qDebug() << "format: " << frame->format;
                AVPixelFormat fmt = static_cast<AVPixelFormat>(frame->format);
                const char* fmt_name = av_get_pix_fmt_name(fmt);
                qDebug() << "The pixel format is " << fmt_name;
                // Convert the image

                double t1 = (double)clock();
                sws_scale(sws_ctx, frame->data, frame->linesize, 0, dec_ctx->height,
                          rgb_frame->data, rgb_frame->linesize);
                double t2 = (double)clock();
                qDebug() << "t2 - t1: " << t2 - t1;

                // Save the frame to a file
                // 打开文件
                std::string name = "F:/yuv/frame_" + std::to_string(frame_count) + ".yuv";
                FILE *yuv_file = fopen(name.c_str(), "wb");

                if (!yuv_file) {
                    fprintf(stderr, "Could not open file for writing.\n");
                    return -1;
                }

                // 确保格式正确
                if (frame->format != AV_PIX_FMT_YUV420P10LE) {
                    fprintf(stderr, "Frame format is not yuv420p10le.\n");
                    fclose(yuv_file);
                    return -1;
                }

                // 获取宽度和高度
                int width = frame->width;
                int height = frame->height;

                // 写入数据
                fwrite(frame->data[0], 1, frame->linesize[0] * height, yuv_file); // Y plane
                fwrite(frame->data[1], 1, frame->linesize[1] * (height / 2), yuv_file); // U plane
                fwrite(frame->data[2], 1, frame->linesize[2] * (height / 2), yuv_file); // V plane

                fclose(yuv_file);

                // 更新文件名中的序号
                frame_count++;

                if (frame_count >= 10) {
                    break;
                }
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }

    // Free the YUV frame
    av_free(out_buffer);
    av_free(frame);
    av_free(rgb_frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&pFormatCtx);

    return 0;
}




