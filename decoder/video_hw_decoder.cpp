#include "video_hw_decoder.h"
#include "Logger/logger.h"
#include "core/utils.h"

enum AVPixelFormat hw_pix_fmt;

static enum AVPixelFormat GetHwFormat(AVCodecContext *,
    const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    LOG_DEBUG() << "Failed to get HW surface format.\n";
    return AV_PIX_FMT_NONE;
}

HwDecoder::~HwDecoder()
{
    /* flush the decoder */
    m_isExitDecode = true;
    if (m_decodeThread.joinable()) {
        LOG_DEBUG() << "join thread";
        m_decodeThread.join();
    }
    packet.data = NULL;
    packet.size = 0;
    av_packet_unref(&packet);

    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&input_ctx);
    av_buffer_unref(&hw_device_ctx);
}

int HwDecoder::Init(const std::string& inputName)
{
    //通过你传入的名字来找到对应的硬件解码类型
    auto type = av_hwdevice_find_type_by_name(hwdevice.c_str());

    if (type == AV_HWDEVICE_TYPE_NONE) {
        LOG_DEBUG() << "Available device types:";
        while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            LOG_DEBUG() << av_hwdevice_get_type_name(type);
        return -1;
    }

    /* open the input file */
    if (avformat_open_input(&input_ctx, inputName.c_str(), NULL, NULL) != 0) {
        LOG_DEBUG() << "Cannot open input file " << inputName;
        return -1;
    }

    if (avformat_find_stream_info(input_ctx, NULL) < 0) {
        LOG_DEBUG() << "Cannot find input stream information.";
        return -1;
    }
    AVCodec*         decoder = NULL;
    /* find the video stream information */
    video_ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, (const struct AVCodec **)(&decoder), 0);
    if (video_ret < 0) {
        LOG_DEBUG() << "Cannot find a video stream in the input file";
        return -1;
    }
    video_stream = video_ret;
    //去遍历所有编解码器支持的硬件解码配置 如果和之前你指定的是一样的 那么就可以继续执行了 不然就找不到
    int i = 0;
    while (1) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i++);
        if (!config) {
            LOG_DEBUG() << "Decoder " << decoder->name
                        << " does not support device type " << av_hwdevice_get_type_name(type);
            return -1;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == type) {
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

    auto video           = input_ctx->streams[video_stream];
    AVRational rate = video->avg_frame_rate;
    m_fps           = (double)rate.num / rate.den;
    m_timeInterval  = (double)1000 / m_fps;
    LOG_DEBUG() << "fps: " << m_fps << " init time interval: " << m_timeInterval;

    if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0)
        return -1;
    //填入回调函数 通过这个函数 编解码器能够知道显卡支持的像素格式
    decoder_ctx->get_format = GetHwFormat;

    if (HwDecoderInit(decoder_ctx, type) < 0) {
        return -1;
    }

    //绑定完成后 打开编解码器
     if ((video_ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
         fprintf(stderr, "Failed to open codec for stream #%u\n", video_stream);
         return -1;
     }
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

int HwDecoder::Restart() {
    Canon::VideoKeyFrame beginFrame;
    beginFrame.posOffset = 0;
    beginFrame.timestamp = 0;
    StartFromKeyFrameAsync(beginFrame);
    return 1;
}

int HwDecoder::Start() {
    m_isExitDecode = false;
    m_decodeType.store(DecodeType::ALL_FRAME);
    m_decodeThread = std::thread(&HwDecoder::DoWork, this);
    return 1;
}

void HwDecoder::StartFromKeyFrameAsync(const Canon::VideoKeyFrame keyFrame) {
    std::thread([=]() {
        SetKeyFrame(keyFrame);
    }).detach();
}

void HwDecoder::SetKeyFrame(const Canon::VideoKeyFrame keyFrame) {
    LOG_DEBUG() << "Set key frame, pos: " << keyFrame.posOffset << " timestamp: " << keyFrame.timestamp;

    m_decodeType.store(DecodeType::INIT);
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_decodeThread.joinable()) {
            auto wait_until = std::chrono::steady_clock::now() + std::chrono::seconds(5); // 等待5秒
            // 直到进入wait状态，或者5秒超时
            if (!m_decodeCond.wait_until(lock, wait_until, [this] { return m_isDecoding.load() == false; })) {
                LOG_DEBUG() << "Timeout waiting for thread to exit";
            }
        }
    }
    auto it = m_keyFrameList.find(keyFrame);
    if (it == m_keyFrameList.end()) {
        LOG_DEBUG() << "Key frame timestamp not found.";
    } else {
        int64_t keyFrameIndex = std::distance(m_keyFrameList.begin(), it);
        LOG_DEBUG() << "Found key frame, keyFrameIndex: " << keyFrameIndex;
    }

    int ret = av_seek_frame(input_ctx, video_stream, keyFrame.timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        LOG_DEBUG() << "Failed to seek to key frame.";
        return;
    }
    avcodec_flush_buffers(decoder_ctx); // 仅刷新缓冲区，不重置解码器
    m_decodeType.store(DecodeType::ALL_FRAME);
    m_decodeCond.notify_all();
    m_isDecoding.store(false);
}

void HwDecoder::StateSwitching()
{
    LOG_DEBUG() << "m_keyFrameList size: " << m_keyFrameList.size();
    auto t = m_decodeType.load();
    switch (m_decodeType.load()) {
    case DecodeType::UNKNOWN:
        m_decodeType.store(DecodeType::INIT);
        break;
    case DecodeType::KEY_FRAME:
        m_decodeType.store(DecodeType::INIT);
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
    while (1) {
        if (m_decodeType.load() == DecodeType::INIT) {
            std::unique_lock<std::mutex> lck(m_mutex);
            LOG_DEBUG() << "wait...";
            m_isDecoding.store(false);
            m_decodeCond.wait(lck);
        }
        if (m_isExitDecode) {
            LOG_DEBUG() << "m_isExitDecode: " << m_isExitDecode << ", exit thread";
            break;
        }
        if (video_ret >= 0) {
            if ((video_ret = av_read_frame(input_ctx, &packet)) < 0) { // 解码结束
                video_ret = Decoder(decoder_ctx, &packet); // 最后再解码一次
                LOG_DEBUG() << "ret: " << video_ret << ", enter wait state";
                StateSwitching();
            } else {
                if (video_stream == packet.stream_index) {
                    video_ret = Decoder(decoder_ctx, &packet);
                }
            }
        }
        av_packet_unref(&packet);
    }
}

int HwDecoder::Decoder(AVCodecContext *avctx, AVPacket *packet)
{
    int ret = 0;
    std::string name;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        LOG_DEBUG() << "Error during decoding\n";
        return ret;
    }

    if (m_decodeType.load() == DecodeType::KEY_FRAME) { // 解析关键帧
        if (packet->flags & AV_PKT_FLAG_KEY) {
            Canon::VideoKeyFrame keyFrame;
            keyFrame.posOffset = packet->pos;
            keyFrame.timestamp = packet->dts;
            // LOG_DEBUG() << "keyFrame.posOffset: " << keyFrame.posOffset
            //             << ", keyFrame.timestamp: " << keyFrame.timestamp
            //             << ", stream_idx: " << packet->stream_index;
            m_keyFrameList.push_back(keyFrame);
        }
        while(1) {
            AVFrame *frame = av_frame_alloc();
            auto ret = avcodec_receive_frame(avctx, frame);
            av_frame_free(&frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                return 0;
            } else if (ret < 0) {
                LOG_DEBUG() << "Error while decoding";
                if (ret < 0)
                    return ret;
            }
        }
    } else if (m_decodeType.load() == DecodeType::ALL_FRAME) {
        AVFrame *frame = NULL, *sw_frame = NULL;
        AVFrame *tmp_frame = NULL;
        while (m_decodeType.load() == DecodeType::ALL_FRAME) {
            if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
                LOG_DEBUG() << "Can not alloc frame";
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
                LOG_DEBUG() << "Error while decoding";
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
            } else {
                tmp_frame = frame;
            }
            // LOG_DEBUG() << "format: " << av_get_pix_fmt_name((AVPixelFormat)tmp_frame->format);
            if (m_decodeType.load() == DecodeType::ALL_FRAME) {
                Canon::VideoFrame* videoFrame = new  Canon::VideoFrame;
                // 用户需手动释放
                videoFrame->frame = new uint8_t[tmp_frame->width * tmp_frame->height * 4];
                videoFrame->pts = tmp_frame->pts;
                videoFrame->width = tmp_frame->width;
                videoFrame->height = tmp_frame->height;
                videoFrame->timeBase = tmp_frame->time_base;
                convert_hardware_yuv_to_rgba(tmp_frame->data[0],
                                             tmp_frame->data[1],
                                             videoFrame->frame,
                                             tmp_frame->width,
                                             tmp_frame->height,
                                             (AVPixelFormat)tmp_frame->format);
                m_frameList.PushBack(std::move(videoFrame));
            }
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
        }
    }

    return 0;
}
