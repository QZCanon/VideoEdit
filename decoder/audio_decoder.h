#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <QObject>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

// #include "task_runner/task_runner.hpp"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class AudioDecoder : public QObject
{
    Q_OBJECT
public:
    explicit AudioDecoder(QObject *parent = nullptr);
    ~AudioDecoder();

    int Init();

    void DoWork();

signals:

private:
    // const std::string filename = "F:/大疆/拍摄/0830/DJI_20240830191244_0060_D.MP4";
    const std::string filename = "F:/test.mp4";
    std::thread m_thread;
    bool m_isRun = false;

private:
    std::string outfilename = "F:/out2.mp3";
    AVCodecContext *codecCtx = NULL;
    AVFormatContext *fmt_ctx = NULL;
    int stream_index = -1;
};

#endif // AUDIO_DECODER_H
