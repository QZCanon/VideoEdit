#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <QObject>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <QThread>
#include <QFile>

// #include "task_runner/task_runner.hpp"
#include "audioplayer.h"

extern "C" {
// ffmpeg
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

// SDL
// #include "SDL2/SDL.h"
}

class AudioDecoder : public QThread
{
    Q_OBJECT
public:
    explicit AudioDecoder(QObject *parent = nullptr);
    ~AudioDecoder();

    int Init();

    void DoWork();

    void run() override;

public:
    bool m_isRun = false;
    AVCodecContext *codecCtx = NULL;

signals:
    void AudioSignal(AVFrame*);

public slots:

private:
#ifdef Q_OS_WIN
    // const std::string filename = "F:/大疆/拍摄/0830/DJI_20240830191244_0060_D.MP4";
    const std::string filename = "F:/test.mp4";
    std::string outfilename = "F:/out2.mp3";
#elif defined(Q_OS_MAC)
    // std::string filename = "/Users/qinzhou/workspace/test/input_file.mp4";
    std::string filename = "/Users/qinzhou/workspace/test/dage.mp4";
    std::string outfilename = "/Users/qinzhou/workspace/test/out11.mp3";
#endif
    std::thread m_thread;
private:
    AVFormatContext *fmt_ctx = NULL;
    int stream_index = -1;
    AudioPlayer* m_audioPlayer;

    QBuffer *buffer;
    void stopAudioOutput();
};

#endif // AUDIO_DECODER_H
