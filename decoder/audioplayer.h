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

#include "core/types.h"
#include "core/atomic_vector.hpp"
#include "audio_decoder.h"

class AudioPlayer : public QObject {
    Q_OBJECT
public:
    AudioPlayer(std::string& fileName, QObject *parent = nullptr) : QObject(parent) {
        InitAudioDecoder(fileName);
        InitAudio();
        Play();
    }

    void Play(const uint64_t time = 0)
    {
        GetAudioData(time);
        m_buffer.open(QIODevice::ReadWrite);
        m_buffer.write(m_audioBuffer);
        m_buffer.seek(0);
        m_audioSink->start(&m_buffer);
    }

private:
    static void AudioDataCallBack(void* self, Canon::AudioData data)
    {
        AudioPlayer* player = (AudioPlayer*)self;
        // player->m_audioBuffer.append(data.data);
        player->m_bufList.push_back(data);
    }

    void InitAudioDecoder(const std::string& fileName)
    {
        m_audioDecoder = std::make_unique<AudioDecoder>();
        m_audioDecoder->SetAudioDataCB(AudioPlayer::AudioDataCallBack, (void*)this);
        m_audioDecoder->Init(fileName);
    }

    void InitAudio()
    {
        m_fileCreateTime = m_audioDecoder->GetFileCreateTime();
        m_audioFormat.setSampleRate(m_audioDecoder->GetSampleRate());
        m_audioFormat.setChannelCount(m_audioDecoder->GetChannels());
        m_audioFormat.setSampleFormat(QAudioFormat::Float);

        QAudioDevice info = QMediaDevices::defaultAudioOutput();
        if (!info.isFormatSupported(m_audioFormat)) {
            qWarning() << "Default format not supported, trying to find a supported format.";
            if (!info.isFormatSupported(m_audioFormat)) {
                qWarning() << "No supported format found.";
            }
        }

        m_audioSink = new QAudioSink(m_audioFormat, this);
        m_audioSink->setVolume(0.5);

        connect(m_audioSink, &QAudioSink::stateChanged,
                this,      &AudioPlayer::AudioStateChanged);
    }


    void GetAudioData(const uint64_t time = 0)
    {
        Canon::AudioData data;
        data.timestamp = time;
        if (time > 0) {
            auto it = m_bufList.find(data);
            for (;it != m_bufList.end(); ++it) {
                m_audioBuffer.append(it->data);
            }
        } else {
            for (auto it = m_bufList.begin(); it != m_bufList.end(); ++it) {
                m_audioBuffer.append(it->data);
            }
        }
    }

public slots:
    void AudioStateChanged(QtAudio::State state)
    {
        Q_UNUSED(state);
    }

private:
    QAudioFormat m_audioFormat;
    QAudioSink*  m_audioSink{nullptr};
    QBuffer      m_buffer;
    QFile        m_sourceFile;
    uint64_t     m_fileCreateTime;

    AtomicVector<Canon::AudioData> m_bufList;     // 单帧音频数据
    QByteArray                    m_audioBuffer; // 所有的音频数据

    std::unique_ptr<AudioDecoder> m_audioDecoder{nullptr};
};
#endif // AUDIOPLAYER_H
