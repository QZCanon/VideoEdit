#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QDebug>
#include <QThread>
#include <QMediaFormat>
#include <QMediaDevices>
#include <QFile>
#include <core/types.h>

extern "C" {
#include "libavutil/frame.h"
#include <libavcodec/avcodec.h>
}

class AudioPlayer : public QObject {
    Q_OBJECT
public:
    AudioPlayer(QObject *parent = nullptr) : QObject(parent) {
        audioFormat.setSampleRate(48000);
        audioFormat.setChannelCount(2);
        audioFormat.setSampleFormat(QAudioFormat::Float);

        QAudioDevice info = QMediaDevices::defaultAudioOutput();
        if (!info.isFormatSupported(audioFormat)) {
            qWarning() << "Default format not supported, trying to find a supported format.";
            if (!info.isFormatSupported(audioFormat)) {
                qWarning() << "No supported format found.";
            }
        }

        audioSink = new QAudioSink(audioFormat, this);
        audioSink->setVolume(0.1);

        connect(this, &AudioPlayer::frameDecoded, this, &AudioPlayer::playFrame);
        connect(audioSink, &QAudioSink::stateChanged,
                this,      &AudioPlayer::AudioStateChanged);
    }

    void SetCreateTime(uint64_t createTime) {
        m_fileCreateTime = createTime;
    }

    void Decode(AVCodecContext* ctx, AVFrame *frame, int data_size) {
        QByteArray arr;
        AudioData  audioData;
        int64_t    pts_in_us      = frame->pts;
        double     pts_in_seconds = av_q2d(frame->time_base) * pts_in_us;
        uint64_t   timestamp      = (uint64_t)pts_in_seconds + m_fileCreateTime;


        for (int i = 0; i < frame->nb_samples; i++) {
            for (int ch = 0; ch < ctx->ch_layout.nb_channels; ch++) {
                m_audioBuffer.append((const char*)(frame->data[ch] + data_size * i), data_size);
                arr.append((const char*)(frame->data[ch] + data_size * i), data_size);
            }
        }
        audioData.timestamp = timestamp;
        audioData.data      = arr;
        m_bufList.push_back(audioData);
    }

    void Play()
    {
        LOG_DEBUG() << "m_bufList size: " << m_bufList.size();
        playFrame(m_audioBuffer);
    }

private:
    void playFrame(QByteArray frameData) {
        buffer.open(QIODevice::ReadWrite);
        buffer.write(frameData);
        buffer.seek(0);
        audioSink->start(&buffer);
    }

public slots:
    void AudioStateChanged(QtAudio::State state)
    {
        Q_UNUSED(state);
    }

signals:
    void frameDecoded(QByteArray frameData);

private:
    QAudioFormat audioFormat;
    QAudioSink*  audioSink{nullptr};
    QBuffer      buffer;
    QByteArray   m_audioBuffer;
    QFile        sourceFile;
    uint64_t     m_fileCreateTime;

    std::vector<AudioData> m_bufList;
};
#endif // AUDIOPLAYER_H
