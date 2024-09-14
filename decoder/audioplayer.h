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
#include "task_runner/task_runner.hpp"

enum class AudioPlayState {
    AUDIO_NORMAL,           // 正常
    AUDIO_ALREADY_PLAY,     // 已经播放
    AUDIO_BUFFER_EMPTY,     // 音频缓冲区为空
    AUDIO_TIMESTAME_ERROR,  // 时间戳错误（音视频不能同步上)
    UNKNOWN,                // 为止状态
};

class AudioPlayer : public QObject {
    Q_OBJECT
public:
    AudioPlayer(std::string& fileName,
                TaskRunner* runner = nullptr,
                QObject *parent = nullptr)
        : QObject(parent)
        , m_runner(runner)
    {
        InitAudioDecoder(fileName);
        InitAudio();
        if (!m_task) {
            m_task = CREATE_TASK_OCJECT();
        }
    }

    AudioPlayState Play(const uint64_t time = 0)
    {
        if (!m_isPlay.load()) {
            m_isPlay.store(true);
        } else {
            return AudioPlayState::AUDIO_ALREADY_PLAY;
        }
        auto ret = GetAudioData(time);
        if (ret != AudioPlayState::AUDIO_NORMAL) {
            m_isPlay.store(false); // 当播放状态不正常时，下一次可再同步
        }
        m_buffer.open(QIODevice::ReadWrite);
        m_buffer.write(m_audioBuffer);
        m_buffer.seek(0);
        m_audioSink->start(&m_buffer);
        emit AudioStarted();
        return ret;
    }

    void Replay(const uint64_t time = 0){
        m_isPlay.store(false);
        Play(time);
    }

    auto IsPlay()
    {
        return m_isPlay.load();
    }

signals:
    void AudioStarted();

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

    AudioPlayState GetAudioData(const uint64_t time = 0)
    {
        Canon::AudioData data;
        data.timestamp = time;
        if (m_bufList.size() == 0) {
            LOG_DEBUG() << "error: audio buffer is empty";
            return AudioPlayState::AUDIO_BUFFER_EMPTY;
        }
        if (time > 0) {
            auto it = m_bufList.find(data);
            if (it == m_bufList.end()) {
                LOG_DEBUG() << "not sync audio, video frame timestame: " << time
                            << ", the audio time range [" << m_bufList.begin()->timestamp << ", "
                            << (m_bufList.end() - 1)->timestamp;
                return AudioPlayState::AUDIO_TIMESTAME_ERROR;
            }
            LOG_DEBUG() << "video and audio sync successful , video timestame: " << time
                        << ", the timestamp: " << it->timestamp;
            for (;it != m_bufList.end(); ++it) {
                m_audioBuffer.append(it->data);
            }
        } else { // < = 0时，重播
            for (auto it = m_bufList.begin(); it != m_bufList.end(); ++it) {
                m_audioBuffer.append(it->data);
            }
        }
        return AudioPlayState::AUDIO_NORMAL;
    }

public slots:
    void AudioStateChanged(QtAudio::State state)
    {
        LOG_DEBUG() << "AudioSink state: "  << state;
        if (state == QtAudio::State::StoppedState) {
            m_audioSink->stop();
        }
    }

private:
    QAudioFormat m_audioFormat;
    QAudioSink*  m_audioSink{nullptr};
    QBuffer      m_buffer;
    QFile        m_sourceFile;
    uint64_t     m_fileCreateTime;
    std::atomic<bool> m_isPlay{false};

    AtomicVector<Canon::AudioData> m_bufList;     // 单帧音频数据
    QByteArray                    m_audioBuffer; // 所有的音频数据

    std::unique_ptr<AudioDecoder> m_audioDecoder{nullptr};

    TaskRunner* m_runner{nullptr};
    TaskSPtr    m_task{nullptr};
};
#endif // AUDIOPLAYER_H
