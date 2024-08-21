#include <cstdint>
#include <cstring>

// Convert p010le to nv12
void convert_p010le_to_nv12(const uint8_t *y_plane, const uint8_t *uv_plane, uint8_t *nv12_data, int width, int height) {
    int uv_width = width / 2;
    int uv_height = height / 2;

    // Allocate space for the NV12 data
    uint8_t *nv12_y = nv12_data;
    uint8_t *nv12_uv = nv12_data + width * height; // Start of UV plane

    // Copy Y plane directly, but convert from 10-bit to 8-bit
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int y_index = y * width + x;
            int nv12_y_index = y * width + x;

            // Convert 10-bit to 8-bit
            uint8_t y_val = (y_plane[y_index * 2] << 2) | (y_plane[y_index * 2 + 1] >> 6);
            nv12_y[nv12_y_index] = y_val;
        }
    }

    // Copy UV plane, converting from 10-bit to 8-bit and interleave
    for (int y = 0; y < uv_height; ++y) {
        for (int x = 0; x < uv_width; ++x) {
            int uv_index = y * uv_width + x * 4; // 4 bytes per UV pair (U and V)
            int nv12_uv_index = (y * uv_width + x) * 2;

            // Convert 10-bit to 8-bit
            uint8_t u_val = (uv_plane[uv_index] << 2) | (uv_plane[uv_index + 1] >> 6);
            uint8_t v_val = (uv_plane[uv_index + 2] << 2) | (uv_plane[uv_index + 3] >> 6);

            // Interleave U and V
            nv12_uv[nv12_uv_index] = u_val;
            nv12_uv[nv12_uv_index + 1] = v_val;
        }
    }
}