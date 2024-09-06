#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <QObject>
#include <thread>
#include <QThread>
#include <QFile>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/types.h"

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

typedef void(*ADCallBack)(void*, Canon::AudioData);

class AudioDecoder : public QThread
{
    Q_OBJECT
public:
    explicit AudioDecoder(QObject *parent = nullptr);
    ~AudioDecoder();

    int Init(const std::string& fileName);

    std::string& GetSampltFormat() { return m_sampleFmt; }

    int GetSampleRate() { return m_sampleRate; }

    int GetChannels() { return m_channels; }

    uint64_t GetFileCreateTime() { return m_createTime; }

    void SetAudioDataCB(ADCallBack cb, void* user)
    {
        m_audioDataCB = cb;
        self          = user;
    }

private:
    void DoWork();
    void run() override;

public:
    bool m_isRun = false;
    AVCodecContext *codecCtx = NULL;

private:
    AVFormatContext* fmt_ctx{nullptr};
    int              stream_index{-1};
    std::string      m_sampleFmt;
    int              m_sampleRate{0};
    int              m_channels{0};

// 内部static函数使用，所以定义为public
public:
    uint64_t m_createTime = -1;
    ADCallBack m_audioDataCB{nullptr};
    void* self;
};

#endif // AUDIO_DECODER_H
