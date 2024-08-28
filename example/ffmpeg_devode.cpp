#include <stdio.h>
 
extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
}
 
static AVBufferRef *hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;
static FILE *output_file = NULL;
 
static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
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
 
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
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
 
static int decode_write(AVCodecContext *avctx, AVPacket *packet)
{
	AVFrame *frame = NULL, *sw_frame = NULL;
	AVFrame *tmp_frame = NULL;
	uint8_t *buffer = NULL;
	int size;
	int ret = 0;
 
	ret = avcodec_send_packet(avctx, packet);
	if (ret < 0) {
		fprintf(stderr, "Error during decoding\n");
		return ret;
	}
 
	while (1) {
		if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
			fprintf(stderr, "Can not alloc frame\n");
			ret = AVERROR(ENOMEM);
			goto fail;
		}
 
		ret = avcodec_receive_frame(avctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&frame);
			av_frame_free(&sw_frame);
			return 0;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error while decoding\n");
			goto fail;
		}
 
		if (frame->format == hw_pix_fmt) {
			/* retrieve data from GPU to CPU */
			if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
				fprintf(stderr, "Error transferring the data to system memory\n");
				goto fail;
			}
			tmp_frame = sw_frame;
		}
		else
			tmp_frame = frame;
 
		size = av_image_get_buffer_size(static_cast<AVPixelFormat>(tmp_frame->format), tmp_frame->width, tmp_frame->height, 1);
		buffer = (uint8_t *)av_malloc(size);
		if (!buffer) {
			fprintf(stderr, "Can not alloc buffer\n");
			ret = AVERROR(ENOMEM);
			goto fail;
		}
		ret = av_image_copy_to_buffer(buffer, size,
			(const uint8_t * const *)tmp_frame->data,
			(const int *)tmp_frame->linesize, static_cast<AVPixelFormat>(tmp_frame->format),
			tmp_frame->width, tmp_frame->height, 1);
		if (ret < 0) {
			fprintf(stderr, "Can not copy image to buffer\n");
			goto fail;
		}
 
		if ((ret = fwrite(buffer, 1, size, output_file)) < 0) {
			fprintf(stderr, "Failed to dump raw data.\n");
			goto fail;
		}

        if (ret >= 0 && tmp_frame) {
            static int frame_count = 0;
            // 为保存的图片分配缓冲区
            uint8_t *buffer = nullptr;
            size_t buffer_size = av_image_get_buffer_size(tmp_frame->format, tmp_frame->width, tmp_frame->height, 1);
            buffer = (uint8_t*)av_malloc(buffer_size);

            // 复制帧数据到缓冲区
            ret = av_image_copy_to_buffer(buffer, buffer_size,
                                        (const uint8_t * const *)tmp_frame->data,
                                        (const int *)tmp_frame->linesize, tmp_frame->format,
                                        tmp_frame->width, tmp_frame->height, 1);
            if (ret < 0) {
                fprintf(stderr, "Can not copy image to buffer\n");
                goto fail;
            }

            // 保存图片到文件
            char filename[100];
            snprintf(filename, sizeof(filename), "frame_%03d.yuv", frame_count++);
            FILE *output_file = fopen(filename, "wb");
            if (!output_file) {
                fprintf(stderr, "Failed to open file '%s' for writing.\n", filename);
                ret = AVERROR_UNKNOWN;
                goto fail;
            }
            fwrite(buffer, 1, buffer_size, output_file);
            fclose(output_file);

            // 释放分配的缓冲区
            av_free(buffer);
        }
 
	fail:
		av_frame_free(&frame);
		av_frame_free(&sw_frame);
		av_freep(&buffer);
		if (ret < 0)
			return ret;
	}
}
 
int main(int argc, char *argv[])
{
	AVFormatContext *input_ctx = NULL;
	int video_stream, ret;
	AVStream *video = NULL;
	AVCodecContext *decoder_ctx = NULL;
	AVCodec *decoder = NULL;
	AVPacket packet;
	enum AVHWDeviceType type;
	int i;
 
	if (argc < 4) {
		fprintf(stderr, "Usage: %s <device type> <input file> <output file>\n", argv[0]);
		return -1;
	}
	//通过你传入的名字来找到对应的硬件解码类型
	type = av_hwdevice_find_type_by_name(argv[1]);
	if (type == AV_HWDEVICE_TYPE_NONE) {
		fprintf(stderr, "Device type %s is not supported.\n", argv[1]);
		fprintf(stderr, "Available device types:");
		while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
			fprintf(stderr, " %s", av_hwdevice_get_type_name(type));
		fprintf(stderr, "\n");
		return -1;
	}
 
	/* open the input file */
	if (avformat_open_input(&input_ctx, argv[2], NULL, NULL) != 0) {
		fprintf(stderr, "Cannot open input file '%s'\n", argv[2]);
		return -1;
	}
 
	if (avformat_find_stream_info(input_ctx, NULL) < 0) {
		fprintf(stderr, "Cannot find input stream information.\n");
		return -1;
	}
 
	/* find the video stream information */
	ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, (const struct AVCodec **)(&decoder), 0);
	if (ret < 0) {
		fprintf(stderr, "Cannot find a video stream in the input file\n");
		return -1;
	}
	video_stream = ret;
	//去遍历所有编解码器支持的硬件解码配置 如果和之前你指定的是一样的 那么就可以继续执行了 不然就找不到
	for (i = 0;; i++) {
		const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
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
 
	video = input_ctx->streams[video_stream];
	if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0)
		return -1;
	//填入回调函数 通过这个函数 编解码器能够知道显卡支持的像素格式
	decoder_ctx->get_format = get_hw_format;
 
	if (hw_decoder_init(decoder_ctx, type) < 0)
		return -1;
   //绑定完成后 打开编解码器
	if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
		fprintf(stderr, "Failed to open codec for stream #%u\n", video_stream);
		return -1;
	}
 
	/* open the file to dump raw data */
	output_file = fopen(argv[3], "w+");
 
	/* actual decoding and dump the raw data */
	while (ret >= 0) {
		if ((ret = av_read_frame(input_ctx, &packet)) < 0)
			break;
 
		if (video_stream == packet.stream_index)
			ret = decode_write(decoder_ctx, &packet);
 
 
		av_packet_unref(&packet);
	}
 
	/* flush the decoder */
	packet.data = NULL;
	packet.size = 0;
	ret = decode_write(decoder_ctx, &packet);
	av_packet_unref(&packet);
 
	if (output_file)
		fclose(output_file);
	avcodec_free_context(&decoder_ctx);
	avformat_close_input(&input_ctx);
	av_buffer_unref(&hw_device_ctx);
 
	return 0;
}