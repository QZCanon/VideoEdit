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

extern "C" {
#include "libavutil/frame.h"
#include <libavcodec/avcodec.h>
}

class AudioPlayer : public QObject {
    Q_OBJECT
public:
    AudioPlayer(QObject *parent = nullptr) : QObject(parent) {
        audioFormat.setSampleRate(44100);
        audioFormat.setChannelCount(1);
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

         sourceFile.setFileName("/Users/qinzhou/workspace/test/out11.mp3");
         sourceFile.open(QIODevice::ReadOnly);
         // audioSink->start(&sourceFile);
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

public:
    void Decode(AVCodecContext* ctx, AVFrame *frame, int data_size) {
        qDebug() << "append size: " << frame->nb_samples * ctx->ch_layout.nb_channels;
        for (int i = 0; i < frame->nb_samples; i++) {
            for (int ch = 0; ch < ctx->ch_layout.nb_channels; ch++) {
                m_audioBuffer.append((const char*)(frame->data[ch] + data_size * i), data_size);
            }
        }
    }

    void Play()
    {
        playFrame(m_audioBuffer);
    }

private:
    QAudioFormat audioFormat;
    QAudioSink *audioSink{nullptr};
    QBuffer buffer;
    QByteArray m_audioBuffer;
    int m_playIdx = 0;
    QFile sourceFile;
};
#endif // AUDIOPLAYER_H
