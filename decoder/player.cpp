#include "player.h"

#include "Logger/logger.h"

int PlayerImpl::InitSDL() // INIT SDL
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
       LOG_DEBUG() <<  "SDL_Init Error: " << SDL_GetError();
       return 0;
    }
    return 1;
}

void PlayerImpl::InitAudioDecoder(const std::string& fileName)
{
    m_audioDecoder = std::make_unique<AudioDecoder>();
    m_audioDecoder->SetAudioDataCB(PlayerImpl::AudioDataCallBack, (void*)this);
    m_audioDecoder->Init(fileName);

    // 设置音频格式
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq     = m_audioDecoder->GetSampleRate();
    desiredSpec.format   = AUDIO_F32LSB;
    desiredSpec.channels = m_audioDecoder->GetChannels();
    desiredSpec.samples  = 4096;

    // 打开音频设备
    m_audioDeviceId = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, NULL, 0);
    if (m_audioDeviceId == 0) {
       LOG_DEBUG() << "无法打开音频设备: " << SDL_GetError();
       SDL_Quit();
       return;
    }
    SDL_PauseAudioDevice(m_audioDeviceId, 0);
}

Canon::AudioPlayState PlayerImpl::PlayAudio(const uint64_t time)
{
    if (!m_isPlay.load()) {
        m_isPlay.store(true);
    } else {
        return Canon::AudioPlayState::AUDIO_ALREADY_PLAY;
    }
    auto ret = GetAudioData(time);
    if (ret != Canon::AudioPlayState::AUDIO_NORMAL) {
        m_isPlay.store(false); // 当播放状态不正常时，下一次可再同步
    }
    SDL_ClearQueuedAudio(m_audioDeviceId);
    SDL_QueueAudio(m_audioDeviceId, m_audioBuffer.data(), m_audioBuffer.size());
    return ret;
}

void PlayerImpl::ReplayAudio(const uint64_t time) {
    m_isPlay.store(false);
    PlayAudio(time);
}

Canon::AudioPlayState PlayerImpl::GetAudioData(const uint64_t time)
{
    Canon::AudioData data;
    data.timestamp = time;
    if (m_bufList.size() == 0) {
        LOG_DEBUG() << "error: audio buffer is empty";
        return Canon::AudioPlayState::AUDIO_BUFFER_EMPTY;
    }
    if (time > 0) {
        auto it = m_bufList.find(data);
        if (it == m_bufList.end()) {
            LOG_DEBUG() << "not sync audio, video frame timestame: " << time
                        << ", the audio time range [" << m_bufList.begin()->timestamp << ", "
                        << (m_bufList.end() - 1)->timestamp;
            return Canon::AudioPlayState::AUDIO_TIMESTAME_ERROR;
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
    return Canon::AudioPlayState::AUDIO_NORMAL;
}

void PlayerImpl::InitVideoDecoder(const std::string& fileName, QWidget* paintUI)
{
    LOG_DEBUG() << "paintUI w: " << paintUI->width() << ", h: " << paintUI->height();
    m_decoder       = new HwDecoder(fileName);
    m_decoder->Start(); // ???
    m_window        = SDL_CreateWindowFrom((void*)paintUI->winId());
    m_renderer      = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_TARGETTEXTURE);
    m_videoPlayTask = CREATE_TASK_OCJECT();
    uint64_t sleep  = 1000000 / m_decoder->GetFileFPS();
    LOG_DEBUG() << "task sleep: " << sleep;
    m_videoPlayTask->SetTaskSleepTime(sleep);
    m_videoPlayTask->SetTaskFunc(std::bind(&PlayerImpl::VideoTaskFunc, this));
}

void PlayerImpl::PlayVideo()
{
    if (m_runner) {
        m_runner->AddTask(m_videoPlayTask);
    }
}

void PlayerImpl::VideoTaskFunc()
{
    m_decoder->BufferWait(); // 为空时应该wait
    Canon::VideoFrame* frameData = m_decoder->GetFrame();
    if (!frameData->frame) {
        return;
    }
    m_videoPlayTask->SetTaskSleepTime(frameData->duration);
    // 创建 SDL 纹理
    if (!m_texture) {
        m_texture = SDL_CreateTexture(m_renderer,
                                      SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      frameData->width,
                                      frameData->height);
        SDL_Rect topLeftViewport = {0, 0, 256, 256};
        SDL_RenderSetViewport(m_renderer, &topLeftViewport);
        SDL_RenderSetLogicalSize(m_renderer, frameData->width, frameData->height); // 设置图像宽高，SDL会自动根据比例缩放
    }
    SDL_Rect dstRect = {0, 0, 256, 256};
    SDL_UpdateTexture(m_texture, NULL, frameData->frame, frameData->width * 4);

    // 渲染纹理到窗口
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    // SDL_RenderCopy(m_renderer, m_texture, NULL, &dstRect);
    SDL_RenderPresent(m_renderer);

    // 清理资源
    // SDL_DestroyTexture(m_texture);
    if (frameData) {
        if (frameData->frame) {
            delete frameData->frame;
            frameData->frame = nullptr;
        }
        delete frameData;
        frameData = nullptr;
    }
}
