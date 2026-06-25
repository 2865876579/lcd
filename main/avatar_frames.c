#include "avatar_frames.h"

extern const uint8_t avatar_frame_0_bin_start[] asm("_binary_avatar_frame_0_bin_start");
extern const uint8_t avatar_frame_1_bin_start[] asm("_binary_avatar_frame_1_bin_start");
extern const uint8_t avatar_frame_2_bin_start[] asm("_binary_avatar_frame_2_bin_start");
extern const uint8_t avatar_frame_3_bin_start[] asm("_binary_avatar_frame_3_bin_start");
extern const uint8_t avatar_frame_4_bin_start[] asm("_binary_avatar_frame_4_bin_start");
extern const uint8_t avatar_frame_5_bin_start[] asm("_binary_avatar_frame_5_bin_start");
extern const uint8_t avatar_frame_6_bin_start[] asm("_binary_avatar_frame_6_bin_start");
extern const uint8_t avatar_frame_7_bin_start[] asm("_binary_avatar_frame_7_bin_start");
extern const uint8_t avatar_frame_8_bin_start[] asm("_binary_avatar_frame_8_bin_start");
extern const uint8_t avatar_frame_9_bin_start[] asm("_binary_avatar_frame_9_bin_start");
extern const uint8_t avatar_frame_10_bin_start[] asm("_binary_avatar_frame_10_bin_start");
extern const uint8_t avatar_frame_11_bin_start[] asm("_binary_avatar_frame_11_bin_start");

#define AVATAR_FRAME_DSC(id)                         \
    const lv_img_dsc_t avatar_frame_##id = {         \
        .header.always_zero = 0,                     \
        .header.w = AVATAR_FRAME_W,                  \
        .header.h = AVATAR_FRAME_H,                  \
        .data_size = AVATAR_FRAME_W * AVATAR_FRAME_H * 2, \
        .header.cf = LV_IMG_CF_TRUE_COLOR,           \
        .data = avatar_frame_##id##_bin_start,       \
    }

AVATAR_FRAME_DSC(0);
AVATAR_FRAME_DSC(1);
AVATAR_FRAME_DSC(2);
AVATAR_FRAME_DSC(3);
AVATAR_FRAME_DSC(4);
AVATAR_FRAME_DSC(5);
AVATAR_FRAME_DSC(6);
AVATAR_FRAME_DSC(7);
AVATAR_FRAME_DSC(8);
AVATAR_FRAME_DSC(9);
AVATAR_FRAME_DSC(10);
AVATAR_FRAME_DSC(11);
