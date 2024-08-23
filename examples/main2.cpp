#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

void convert_hardware_yuv_to_rgba(const uint8_t *y_plane, const uint8_t *uv_plane, 
                                  uint8_t *rgba_frame, int width, int height, 
                                  AVPixelFormat hw_pix_fmt) {
    // Initialize SwsContext for format conversion
    SwsContext *sws_ctx = sws_getContext(width, height, hw_pix_fmt,
                                          width, height, AV_PIX_FMT_RGBA,
                                          SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        // Handle error
        return;
    }

    // Prepare source and destination data pointers and line sizes
    uint8_t *src_data[4] = {0};
    int src_linesize[4] = {0};
    uint8_t *dst_data[4] = {rgba_frame};
    int dst_linesize[4] = {4 * width}; // RGBA format has 4 bytes per pixel

    // Set source data pointers and line sizes
    src_data[0] = (uint8_t *)y_plane;  // Y plane
    src_linesize[0] = width * 2;       // For 10-bit YUV, 2 bytes per pixel in Y plane

    src_data[1] = (uint8_t *)uv_plane;  // UV plane
    src_linesize[1] = width * 2;        // For 10-bit YUV, 2 bytes per pixel in UV plane

    // If the UV plane is interleaved, you may need to handle it differently
    // e.g., src_linesize[1] = width; for 4:2:0 formats with single plane

    // Convert the frame
    sws_scale(sws_ctx, (const uint8_t * const *)src_data, src_linesize,
              0, height, dst_data, dst_linesize);

    // Clean up
    sws_freeContext(sws_ctx);
}
