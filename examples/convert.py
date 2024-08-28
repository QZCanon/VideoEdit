import numpy as np

def p010le_to_nv12(p010le, width, height):
    # 提取 P010LE 的 Y 平面和 UV 平面
    y_plane = p010le[:height * width * 2].reshape(height, width, 2)
    uv_plane = p010le[height * width * 2:].reshape(height // 2, width, 4)

    # 将 Y 平面的 10 位数据转换为 8 位
    y_plane_8bit = (y_plane[..., 0] >> 2).astype(np.uint8)

    # 将 UV 平面的 10 位数据转换为 8 位，并交错存储 U 和 V 分量
    u_plane_8bit = (uv_plane[..., 0] >> 2).astype(np.uint8)
    v_plane_8bit = (uv_plane[..., 2] >> 2).astype(np.uint8)
    uv_plane_8bit = np.empty((height // 2, width), dtype=np.uint8)
    uv_plane_8bit[:, 0::2] = u_plane_8bit
    uv_plane_8bit[:, 1::2] = v_plane_8bit

    # 合并 Y 平面和 UV 平面
    nv12 = np.concatenate((y_plane_8bit.flatten(), uv_plane_8bit.flatten()))

    return nv12
