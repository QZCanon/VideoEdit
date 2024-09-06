#include "video_hw_decoder.h"
#include "Logger/logger.h"
#include "core/utils.h"

#include "core/InstanceManager.h"

enum AVPixelFormat hw_pix_fmt;

static enum AVPixelFormat GetHwFormat(AVCodecContext *,
    const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

HwDecoder::~HwDecoder()
{
    /* flush the decoder */
    isExitDecode = true;
    if (th.joinable()) {
        LOG_DEBUG() << "join thread";
        th.join();
    }
    packet.data = NULL;
    packet.size = 0;
    av_packet_unref(&packet);

    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&input_ctx);
    av_buffer_unref(&hw_device_ctx);
}

int HwDecoder::Init()
{
    //通过你传入的名字来找到对应的硬件解码类型
    type = av_hwdevice_find_type_by_name(hwdevice.c_str());

    if (type == AV_HWDEVICE_TYPE_NONE) {
        // fprintf(stderr, "Device type %s is not supported.\n", argv[1]);
        fprintf(stderr, "Available device types:");
        while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            fprintf(stderr, " %s", av_hwdevice_get_type_name(type));
        fprintf(stderr, "\n");
        return -1;
    }

    /* open the input file */
    if (avformat_open_input(&input_ctx, inputName.c_str(), NULL, NULL) != 0) {
        // fprintf(stderr, "Cannot open input file '%s'\n", argv[2]);
        return -1;
    }

    if (avformat_find_stream_info(input_ctx, NULL) < 0) {
        fprintf(stderr, "Cannot find input stream information.\n");
        return -1;
    }

    /* find the video stream information */
    video_ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, (const struct AVCodec **)(&decoder), 0);
    if (video_ret < 0) {
        fprintf(stderr, "Cannot find a video stream in the input file\n");
        return -1;
    }
    video_stream = video_ret;
    //去遍历所有编解码器支持的硬件解码配置 如果和之前你指定的是一样的 那么就可以继续执行了 不然就找不到
    int i = 0;
    while (1) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i++);
        if (!config) {
            fprintf(stderr, "Decoder %s does not support device type %s.\n",
                decoder->name, av_hwdevice_get_type_name(type));
            return -1;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == type) {
            //把硬件支持的像素格式设置进去
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }

    if (!(decoder_ctx = avcodec_alloc_context3(decoder)))
        return AVERROR(ENOMEM);

    if (input_ctx->metadata) {
        auto* time_ = av_dict_get(input_ctx->metadata, "creation_time", nullptr, 0);
        if (time_) {
            m_createTime = DJIVideoCreateTimeConvert(time_->value);
            LOG_DEBUG() << "create time: " << time_->value << ", convert[+8h]: " << m_createTime;
        } else {
            LOG_DEBUG() << "time is null, from the api [av_dict_get]";
        }
    } else {
        LOG_DEBUG() << "metadata is null";
    }

    video = input_ctx->streams[video_stream];
    AVRational rate = video->avg_frame_rate;
    m_fps = rate.num / rate.den + 0.5;
    LOG_DEBUG() << "fps: " << m_fps;

    if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0)
        return -1;
    //填入回调函数 通过这个函数 编解码器能够知道显卡支持的像素格式
    decoder_ctx->get_format = GetHwFormat;

    if (HwDecoderInit(decoder_ctx, type) < 0)
        return -1;

    OpenCodec();
    StartThread();
    return 1;
}

int HwDecoder::OpenCodec() {
   //绑定完成后 打开编解码器
    if ((video_ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
        fprintf(stderr, "Failed to open codec for stream #%u\n", video_stream);
        return -1;
    }
    return 1;
}

int HwDecoder::StartThread() {
    isExitDecode = false;
    m_decodeType.store(DecodeType::KEY_FRAME);
    th = std::thread(&HwDecoder::DoWork, this);
    return 1;
}

int HwDecoder::HwDecoderInit(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;
    //创建硬件设备信息上下文
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
        NULL, NULL, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    //绑定编解码器上下文和硬件设备信息上下文
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

void HwDecoder::StateSwitching()
{
    LOG_DEBUG() << "m_keyFrameList size: " << m_keyFrameList.size();
    auto t = m_decodeType.load();
    switch (m_decodeType.load()) {
    case DecodeType::UNKNOWN:
        m_decodeType.store(DecodeType::KEY_FRAME);
        break;
    case DecodeType::KEY_FRAME:
        m_decodeType.store(DecodeType::ALL_FRAME);
        break;
    case DecodeType::ALL_FRAME:
        m_decodeType.store(DecodeType::INIT);
        break;
    default:
        break;
    }
     LOG_DEBUG() << "before: " << (int)t << ", after: " << (int)m_decodeType.load();
}

void HwDecoder::DoWork()
{
    /* actual decoding and dump the raw data */
    while (1) {
        if (m_decodeType.load() == DecodeType::INIT) {
            std::unique_lock<std::mutex> lck(m_mutex);
            LOG_DEBUG() << "wait...";
            m_decodeCond.wait(lck);
        }
        if (isExitDecode) {
            LOG_DEBUG() << "isExitDecode: " << isExitDecode << ", exit thread";
            break;
        }
        if (video_ret >= 0) {
            if ((video_ret = av_read_frame(input_ctx, &packet)) < 0) // 解码结束
            {
                video_ret = DecodeWrite(decoder_ctx, &packet);
                // LOG_DEBUG() << "ret: " << video_ret << ", exit thread";
                StateSwitching();
            }

            if (video_stream == packet.stream_index) {
                video_ret = DecodeWrite(decoder_ctx, &packet);
            }
            av_packet_unref(&packet);
        } else {
            video_ret = DecodeWrite(decoder_ctx, &packet);
            LOG_DEBUG() << "ret: " << video_ret << ", exit thread";
            // break;
            StateSwitching();
        }
    }
}

int HwDecoder::DecodeWrite(AVCodecContext *avctx, AVPacket *packet)
{
    int ret = 0;
    std::string name;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        LOG_DEBUG() << "Error during decoding\n";
        return ret;
    }

    if (packet->flags & AV_PKT_FLAG_KEY && m_decodeType.load() == DecodeType::KEY_FRAME) {
        // LOG_DEBUG() << "find key frame, timestame: " << packet->dts << ", offset: " << packet->pos;
        Canon::VideoKeyFrame keyFrame;
        keyFrame.posOffset = packet->pos;
        keyFrame.timestamp = packet->dts;
        m_keyFrameList.push_back(keyFrame);
        return 1;
    }
    while (1) {
        AVFrame *frame = NULL, *sw_frame = NULL;
        AVFrame *tmp_frame = NULL;
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            LOG_DEBUG() << "Can not alloc frame\n";
            ret = AVERROR(ENOMEM);
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            if (ret < 0)
                return ret;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            LOG_DEBUG() << "Error while decoding\n";
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            if (ret < 0)
                return ret;
        }
        AVRational time_base = input_ctx->streams[video_stream]->time_base;
        // LOG_DEBUG() << "hw_pix_fmt: " << av_get_pix_fmt_name((AVPixelFormat)hw_pix_fmt);
        if (frame->format == hw_pix_fmt) {
            /* retrieve data from GPU to CPU */
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                LOG_DEBUG() << "Error transferring the data to system memory";
                av_frame_free(&frame);
                av_frame_free(&sw_frame);
                if (ret < 0)
                    return ret;
            }
            sw_frame->width     = frame->width;
            sw_frame->height    = frame->height;
            sw_frame->pts       = frame->pts;
            sw_frame->pkt_dts   = frame->pkt_dts;
            sw_frame->pict_type = frame->pict_type;
            sw_frame->time_base = time_base;
            tmp_frame           = sw_frame;
            av_frame_free(&frame);
        } else {
            tmp_frame = frame;
            av_frame_free(&sw_frame);
        }

        // LOG_DEBUG() << "format: " << av_get_pix_fmt_name((AVPixelFormat)tmp_frame->format);
        m_frameList.PushBack(std::move(tmp_frame));

        // av_frame_free(&tmp_frame);
    }

    return 0;
}
