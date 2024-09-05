#include "audio_decoder.h"

#include "Logger/logger.h"

#include <QBuffer>
#include <QAudioFormat>
#include <QMediaFormat>
#include <QAudioSink>
#include <QMediaPlayer>
#include <QMediaDevices>
#include "core/utils.h"

#define SAVE_AUDIO 0

AudioDecoder::AudioDecoder(QObject *parent)
    : QThread(parent)
{
    // m_audioPlayer = new AudioPlayer;
}

AudioDecoder::~AudioDecoder()
{
    m_isRun = false;
}

#if SAVE_AUDIO
static int GetFormat(const char **fmt, enum AVSampleFormat sample_fmt)
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
#endif


static void decode(AVCodecContext *dec_ctx,
                   AVFormatContext* fmtCtx,
                   AVPacket *pkt,
                   FILE *outfile,
                   AudioDecoder* decode)
{
    /* send the packet with the compressed data to the decoder */
    int ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        LOG_DEBUG() << "Error submitting the packet to the decoder";
        return;
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        auto* frame = av_frame_alloc();
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            LOG_DEBUG() << "Error during decoding";
            return;
        }

        int data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            LOG_DEBUG() << "Failed to calculate data size";
            return;
        }
        AVRational time_base = fmtCtx->streams[pkt->stream_index]->time_base;
        frame->time_base = time_base;
        // player->Decode(dec_ctx, frame, data_size);

        // QByteArray audioBuffer;

        AudioData  audioData;
        int64_t    pts_in_us      = frame->pts;
        double     pts_in_seconds = av_q2d(frame->time_base) * pts_in_us;
        audioData.timestamp       = (uint64_t)pts_in_seconds + decode->m_createTime;
        for (int i = 0; i < frame->nb_samples; i++) {
            for (int ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++) {
               audioData.data.append((const char*)(frame->data[ch] + data_size * i), data_size);
            }
        }
        av_frame_free(&frame);

        if (decode->m_audioDataCB) {
            decode->m_audioDataCB(decode->self, audioData);
        }

        // LOG_DEBUG() << "frame: " << av_get_sample_fmt_name((AVSampleFormat)frame->format);
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
        for (int i = 0; i < frame->nb_samples; i++)
            for (int ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++)
                fwrite(frame->data[ch] + data_size*i, 1, data_size, outfile);
#else
        Q_UNUSED(outfile);
#endif
    }
}

int AudioDecoder::Init(const std::string& fileName)
{
    /* Open input file, and retrieve the file's format */
    if (avformat_open_input(&fmt_ctx, fileName.c_str(), NULL, NULL) < 0) {
        LOG_DEBUG() <<  "Could not open input file '%s'\n" << fileName.c_str();
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
    m_sampleFmt = std::string(av_get_sample_fmt_name(sample_fmt));
    m_sampleRate = codecCtx->sample_rate;
    m_channels = codecCtx->ch_layout.nb_channels;


    LOG_DEBUG() << "Sample Format: " << m_sampleFmt;
    LOG_DEBUG() << "Sample Rate: " << m_sampleRate;
    LOG_DEBUG() << "Channels: " << m_channels;

    /* Open codec */
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        LOG_DEBUG() <<  "Could not open codec";
        return -1;
    }

    if (fmt_ctx->metadata) {
        auto* time_ = av_dict_get(fmt_ctx->metadata, "creation_time", nullptr, 0);
        if (time_) {
            m_createTime = DJIVideoCreateTimeConvert(time_->value);
            LOG_DEBUG() << "create time: " << time_->value << ", convert[+8h]: " << m_createTime;
        } else {
            LOG_DEBUG() << "time is null, from the api [av_dict_get]";
        }
    } else {
        LOG_DEBUG() << "metadata is null";
    }
    m_isRun = true;
    DoWork();

    return 1;
}

void AudioDecoder::run()
{
    // DoWork();
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

    /* Decode until EOF */
    while (1) {
        if (!m_isRun) {
            break;
        }
        if (av_read_frame(fmt_ctx, pkt) < 0) {
            break;
        }
        if (pkt->stream_index == stream_index) {
            decode(codecCtx, fmt_ctx, pkt, outfile, this);
        }
        av_packet_unref(pkt);
    }

    /* Flush the decoder */
    decode(codecCtx, fmt_ctx, pkt, outfile, this);

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
    if (GetFormat(&fmt, sfmt) < 0)
        goto end;
#if defined(Q_OS_WIN)
    printf("Play the output audio file with the command:\n"
           "ffplay -f %s -ac %d -ar %d %s\n",
           fmt, n_channels, codecCtx->sample_rate,
           outfilename.c_str());
#elif defined(Q_OS_MAC)
    printf("Play the output audio file with the command:\n"
           "ffplay -f %s  -ar %d %s\n",
           fmt,  codecCtx->sample_rate,
           outfilename.c_str());
#endif
end:
    fclose(outfile);
#endif

    avcodec_free_context(&codecCtx);
    av_packet_free(&pkt);
    avformat_close_input(&fmt_ctx);
}
