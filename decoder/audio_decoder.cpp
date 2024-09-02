#include "audio_decoder.h"

#include "Logger/logger.h"

#include <QBuffer>
#include <QAudioFormat>
#include <QMediaFormat>
#include <QAudioSink>
#include <QMediaPlayer>
#include <QMediaDevices>

#define SAVE_AUDIO 0

AudioDecoder::AudioDecoder(QObject *parent)
    : QThread(parent)
{}

AudioDecoder::~AudioDecoder()
{
    m_isRun = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
                              { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
                              { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
                              { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
                              { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
                              { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
                              };
    *fmt = NULL;

    for (uint32_t i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    LOG_DEBUG() << "sample format " << av_get_sample_fmt_name(sample_fmt)
                << " is not supported as output format";
    return -1;
}


static void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame,
                   FILE *outfile)
{
    int i, ch;
    int ret, data_size;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        LOG_DEBUG() << "Error submitting the packet to the decoder";
        return;
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            LOG_DEBUG() << "Error during decoding";
            return;
        }
        // emit AudioSignal(frame);
        // 创建一个QBuffer和QByteArray来存储解码后的音频数据
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        // 将AVFrame中的数据转换为QByteArray
        QByteArray audioData;
        audioData.append((const char*)frame->data[0], frame->linesize[0]);
        if (dec_ctx->ch_layout.nb_channels > 1) {
            audioData.append((const char*)frame->data[1], frame->linesize[1]);
        }

        // 设置音频格式
        QAudioFormat format;
        format.setSampleRate(dec_ctx->sample_rate);
        format.setChannelCount(dec_ctx->ch_layout.nb_channels); // 使用AVCodecContext的channels成员
        format.setSampleFormat(QAudioFormat::Int16);

        // 选择音频输出设备
        QAudioDevice defaultDevice = QMediaDevices::defaultAudioOutput();
        QAudioSink audioSink(defaultDevice, format);
        audioSink.setVolume(0.5);
        // 开始播放
        audioSink.start(&buffer);
        audioSink.stop();


        // LOG_DEBUG() << "frame: " << av_get_pix_fmt_name((AVPixelFormat)frame->format);
        // AVSampleFormat frame_sample_fmt = (AVSampleFormat)frame->format;
        // int frame_sample_rate = frame->sample_rate;
        // LOG_DEBUG() << "Frame Sample Format: " << av_get_sample_fmt_name(frame_sample_fmt);
        // LOG_DEBUG() << "Frame Sample Rate: " << frame_sample_rate;
        // // 检查样本格式是否为 32 位浮点数
        // if (frame_sample_fmt == AV_SAMPLE_FMT_FLT) {
        //     LOG_DEBUG() << "Decoded audio is in 32-bit float format.";
        // } else {
        //     LOG_DEBUG() << "Decoded audio is not in 32-bit float format.";
        // }
#if SAVE_AUDIO
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            LOG_DEBUG() << "Failed to calculate data size";
            return;
        }
        for (i = 0; i < frame->nb_samples; i++)
            for (ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++)
                fwrite(frame->data[ch] + data_size*i, 1, data_size, outfile);
#endif
    }
}

void AudioDecoder::RecAudio(AVFrame*)
{

}


// 假设 audioBuffer 是一个包含音频样本的缓冲区
// Uint8* audioBuffer;
// int audioBufferSize;

// 音量级别，范围从0.0到1.0
float volume = 0.1f;

// 在播放音频之前调整音量
void AdjustVolume(Uint8* buffer, int bufferSize) {
    // 计算每个样本的音量调整值
    int volumeScale = static_cast<int>(volume * 128);

    // 遍历缓冲区，调整每个样本的音量
    for (int i = 0; i < bufferSize; ++i) {
        // 对于16位音频，每个样本是2字节
        if (buffer[i] & 0x80) {
            buffer[i] = (buffer[i] * volumeScale) / 128;
        }
    }
}

static void AudioCallback(void* userdata, Uint8* stream, int len)
{
    AudioDecoder* self = static_cast<AudioDecoder*>(userdata);
    AVFrame* frame = av_frame_alloc();
        // 解码一帧音频数据
    if (!self || !self->codecCtx) {
        return;
    }

    AdjustVolume(stream, len);

    while (len > 0 && self->m_isRun) {
        int ret = avcodec_receive_frame(self->codecCtx, frame);
        if (ret != 0) {
            // LOG_DEBUG() << "Error during decoding";
            break;
        }
        LOG_DEBUG() << "write...";
        // 将解码后的音频数据写入SDL音频缓冲区
        // int bytesPerSample = av_get_bytes_per_sample(self->codecCtx->sample_fmt);
        // int samplesToCopy = std::min(len / bytesPerSample, frame->nb_samples);
        // memcpy(stream, frame->data[0], samplesToCopy * bytesPerSample);
        // stream += samplesToCopy * bytesPerSample;
        // len -= samplesToCopy * bytesPerSample;
    }

    av_frame_free(&frame);
}

int AudioDecoder::Init()
{
    /* Open input file, and retrieve the file's format */
    if (avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL) < 0) {
        LOG_DEBUG() <<  "Could not open input file '%s'\n" << filename.c_str();
        return -1;
    }

    /* Retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        LOG_DEBUG() <<  "Failed to retrieve input stream information";
        return -1;
    }

    /* Find the first audio stream */
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index = i;
            break;
        }
    }

    if (stream_index == -1) {
        LOG_DEBUG() <<  "No audio stream found in the input file";
        return -1;
    }

    AVStream* stream = fmt_ctx->streams[stream_index];
    AVCodecParameters* codecpar = stream->codecpar;

    /* Find the decoder for the audio stream */
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        LOG_DEBUG() <<  "Codec not found";
        return -1;
    }

    /* Allocate a decoding context */
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOG_DEBUG() <<  "Could not allocate audio codec context";
        return -1;
    }

    /* Copy codec parameters from input stream to output context */
    if (avcodec_parameters_to_context(codecCtx, codecpar) < 0) {
        LOG_DEBUG() <<  "Failed to copy codec parameters to codec context";
        return -1;
    }
    
    // 检查解码上下文的样本格式
    AVSampleFormat sample_fmt = codecCtx->sample_fmt;
    int sample_rate = codecCtx->sample_rate;
    int channels = codecCtx->ch_layout.nb_channels;

    LOG_DEBUG() << "Sample Format: " << av_get_sample_fmt_name(sample_fmt);
    LOG_DEBUG() << "Sample Rate: " << sample_rate;
    LOG_DEBUG() << "Channels: " << channels;

    /* Open codec */
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        LOG_DEBUG() <<  "Could not open codec";
        return -1;
    }

    // m_isRun = true;
    // m_thread = std::thread(&AudioDecoder::DoWork, this);
    // return 1;
    m_isRun = true;
    this->start();
}

void AudioDecoder::run()
{
    LOG_DEBUG();
    DoWork();
}

void AudioDecoder::DoWork()
{
#if SAVE_AUDIO
    /* Open output file */
    FILE *outfile = fopen(outfilename.c_str(), "wb");
    if (!outfile) {
        LOG_DEBUG() <<  "Could not open output file " << outfilename.c_str();
        return;
    }
#else
    FILE *outfile = nullptr;
#endif
    auto* pkt = av_packet_alloc();
    auto* decoded_frame = av_frame_alloc();
    /* Decode until EOF */
    while (1) {
        if (!m_isRun) {
            break;
        }
        if (av_read_frame(fmt_ctx, pkt) < 0) {
            break;
        }
        if (pkt->stream_index == stream_index) {
            decode(codecCtx, pkt, decoded_frame, outfile);
        }
        av_packet_unref(pkt);
    }

    /* Flush the decoder */
    decode(codecCtx, pkt, decoded_frame, outfile);

#if SAVE_AUDIO
    /* Print output PCM information */
    AVSampleFormat sfmt = codecCtx->sample_fmt;

    if (av_sample_fmt_is_planar(sfmt)) {
        const char *packed = av_get_sample_fmt_name(sfmt);
        printf("Warning: the sample format the decoder produced is planar "
               "(%s). This example will output the first channel only.\n",
               packed ? packed : "?");
        sfmt = av_get_packed_sample_fmt(sfmt);
    }

    int n_channels = codecCtx->ch_layout.nb_channels;
    const char *fmt;
    if (get_format_from_sample_fmt(&fmt, sfmt) < 0)
        goto end;

    printf("Play the output audio file with the command:\n"
           "ffplay -f %s -ac %d -ar %d %s\n",
           fmt, n_channels, codecCtx->sample_rate,
           outfilename.c_str());

end:
    fclose(outfile);
#endif

    avcodec_free_context(&codecCtx);
    av_frame_free(&decoded_frame);
    av_packet_free(&pkt);
    avformat_close_input(&fmt_ctx);
}