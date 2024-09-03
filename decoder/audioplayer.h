#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QDebug>
#include <QThread>
#include <QMediaFormat>
#include <QMediaDevices>
extern "C" {
#include "libavutil/frame.h"
#include <libavcodec/avcodec.h>
}

class AudioPlayer : public QObject {
    Q_OBJECT
public:
    AudioPlayer(QObject *parent = nullptr) : QObject(parent) {
        // 初始化音频格式
        audioFormat.setSampleRate(48000); // 以44100Hz为例，根据实际情况调整
        audioFormat.setChannelCount(2); // 假设是双声道
        audioFormat.setSampleFormat(QAudioFormat::Float);

        // 检查格式是否被支持
        QAudioDevice info = QMediaDevices::defaultAudioOutput();
        if (!info.isFormatSupported(audioFormat)) {
            qWarning() << "Default format not supported, trying to find a supported format.";
            // 如果默认格式不被支持，尝试修改参数直到找到一个支持的格式
            audioFormat.setSampleRate(48000); // 尝试使用48000Hz
            if (!info.isFormatSupported(audioFormat)) {
                qWarning() << "No supported format found.";
                // 如果还是找不到支持的格式，可能需要停止程序或通知用户
            }
        }
        // 创建缓冲区和音频设备
        buffer.open(QIODevice::ReadWrite);
        audioSink = new QAudioSink(audioFormat, this);
        audioSink->setVolume(0.5);

        // 开始播放
        connect(this, &AudioPlayer::frameDecoded, this, &AudioPlayer::playFrame);
    }

public slots:
    void playFrame(QByteArray frameData) {
        buffer.write(frameData);
        buffer.seek(0);
        audioSink->start(&buffer);
    }

signals:
    void frameDecoded(QByteArray frameData); // 定义信号

public:
    void decodeAndPlay(AVCodecContext* ctx, AVFrame *frame) {
        // 将AVFrame数据转换为QByteArray
        QByteArray frameData;
        frameData.append((const char*)frame->data[0], frame->linesize[0]);
        if (ctx->ch_layout.nb_channels > 1) {
            frameData.append((const char*)frame->data[1], frame->linesize[1]);
        }

        // 发送信号，通知播放器播放解码后的帧
        emit frameDecoded(frameData);
    }

private:
    QAudioFormat audioFormat;
    QBuffer buffer;
    QAudioSink *audioSink;
};
#endif // AUDIOPLAYER_H
