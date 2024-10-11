#ifndef PLAYER_H
#define PLAYER_H

#include <QBuffer>
#include <QDebug>
#include <QFile>

#include "core/types.h"
#include "core/atomic_vector.hpp"
#include "audio_decoder.h"
#include "task_runner/task_runner.hpp"
#include "video_hw_decoder.h"


#if defined(Q_OS_MAC)
#include "SDL2/SDL.h"
#elif defined(Q_OS_WIN)
#include "SDL.h"
#endif

class PlayerImpl : public QObject
{
    Q_OBJECT
public:
    PlayerImpl(const std::string fileName, QWidget* paintUI, QObject* parent = nullptr)
        : m_paintUI(paintUI)
    {
        Q_UNUSED(parent);
        InitSDL();
        InitAudioDecoder(fileName);
        InitVideoDecoder(fileName, paintUI);
    }

    void SetTaskRunner(TaskRunner* runner) { m_runner = runner; }

    virtual ~PlayerImpl() {}

signals:

public:
    int InitSDL();

    // audio
    void InitAudioDecoder(const std::string& fileName);
    Canon::AudioPlayState PlayAudio(const uint64_t time = 0);
    void ReplayAudio(const uint64_t time = 0);

    // video
    void InitVideoDecoder(const std::string& fileName, QWidget* paintUI);
    void PlayVideo();
    void VideoTaskFunc();

private:
    static void AudioDataCallBack(void* self, Canon::AudioData data)
    {
        PlayerImpl* player = (PlayerImpl*)self;
        player->m_audioBuffer.append(data.data);
        player->m_bufList.push_back(data);
    }

    Canon::AudioPlayState GetAudioData(const uint64_t time = 0);

private:
    QBuffer                         m_buffer;
    QFile                           m_sourceFile;
    uint64_t                        m_fileCreateTime;
    std::atomic<bool>               m_isPlay{false};
    SDL_AudioDeviceID               m_audioDeviceId;
    AtomicVector<Canon::AudioData>  m_bufList;     // 单帧音频数据
    QByteArray                      m_audioBuffer; // 所有的音频数据
    std::unique_ptr<AudioDecoder>   m_audioDecoder{nullptr};
    TaskRunner*                     m_runner{nullptr};
    TaskSPtr                        m_audioPlayTask{nullptr};

    HwDecoder*    m_decoder       = nullptr;
    SDL_Window*   m_window        = nullptr;
    SDL_Renderer* m_renderer      = nullptr;
    SDL_Texture*  m_texture       = nullptr;
    TaskSPtr      m_videoPlayTask = nullptr;
    QWidget*      m_paintUI       = nullptr;
};

#endif // PLAYER_H
