#include <stdbool.h>
#include <stdint.h>

#include "avatar_frames.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcd_ili9488.h"
#include "lvgl.h"

#if LV_COLOR_DEPTH != 16
#error "This display bridge expects LVGL LV_COLOR_DEPTH=16"
#endif

#define LVGL_DRAW_BUF_LINES 24
#define LVGL_TICK_MS 2
#define ANIM_TIMER_MS 40
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *TAG = "screen_lvgl";

static lv_disp_draw_buf_t s_draw_buf;
static lv_disp_drv_t s_disp_drv;
static lv_color_t *s_draw_buf_1;
static uint8_t *s_flush_buf;
static esp_timer_handle_t s_lvgl_tick_timer;
static lv_obj_t *s_avatar_frame_img;

static const lv_img_dsc_t *const s_avatar_frames[] = {
    &avatar_frame_0,
    &avatar_frame_1,
    &avatar_frame_2,
    &avatar_frame_3,
    &avatar_frame_4,
    &avatar_frame_5,
    &avatar_frame_6,
    &avatar_frame_7,
    &avatar_frame_8,
    &avatar_frame_9,
    &avatar_frame_10,
    &avatar_frame_11,
};

static uint8_t rgb5_to_rgb666_byte(uint8_t value)
{
    uint8_t six_bit = (uint8_t)((value << 1) | (value >> 4));
    return (uint8_t)(six_bit << 2);
}

static uint8_t rgb6_to_rgb666_byte(uint8_t value)
{
    return (uint8_t)(value << 2);
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    const int32_t src_w = area->x2 - area->x1 + 1;
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;

    if (x2 < 0 || y2 < 0 || x1 >= LCD_ILI9488_H_RES || y1 >= LCD_ILI9488_V_RES) {
        lv_disp_flush_ready(drv);
        return;
    }

    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 >= LCD_ILI9488_H_RES) {
        x2 = LCD_ILI9488_H_RES - 1;
    }
    if (y2 >= LCD_ILI9488_V_RES) {
        y2 = LCD_ILI9488_V_RES - 1;
    }

    const int w = (int)(x2 - x1 + 1);
    const int h = (int)(y2 - y1 + 1);
    const size_t pixel_count = (size_t)w * h;
    if (pixel_count > (size_t)LCD_ILI9488_H_RES * LVGL_DRAW_BUF_LINES) {
        ESP_LOGE(TAG, "flush area too large: %dx%d", w, h);
        lv_disp_flush_ready(drv);
        return;
    }

    uint8_t *dst = s_flush_buf;
    for (int row = 0; row < h; row++) {
        const lv_color_t *src = color_map +
                                ((size_t)(y1 - area->y1 + row) * src_w) +
                                (x1 - area->x1);
        for (int col = 0; col < w; col++) {
            *dst++ = rgb5_to_rgb666_byte(LV_COLOR_GET_R(src[col]));
            *dst++ = rgb6_to_rgb666_byte(LV_COLOR_GET_G(src[col]));
            *dst++ = rgb5_to_rgb666_byte(LV_COLOR_GET_B(src[col]));
        }
    }

    esp_err_t err = lcd_ili9488_draw_rgb666_image((int)x1, (int)y1, w, h, s_flush_buf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "flush failed: %s", esp_err_to_name(err));
    }

    lv_disp_flush_ready(drv);
}

static void lvgl_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(LVGL_TICK_MS);
}

static esp_err_t lvgl_port_init(void)
{
    lv_init();

    const size_t pixel_count = LCD_ILI9488_H_RES * LVGL_DRAW_BUF_LINES;
    s_draw_buf_1 = heap_caps_malloc(pixel_count * sizeof(lv_color_t),
                                    MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    ESP_RETURN_ON_FALSE(s_draw_buf_1 != NULL, ESP_ERR_NO_MEM, TAG, "lvgl draw buffer");

    s_flush_buf = heap_caps_malloc(pixel_count * 3,
                                   MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    ESP_RETURN_ON_FALSE(s_flush_buf != NULL, ESP_ERR_NO_MEM, TAG, "lvgl rgb666 buffer");

    lv_disp_draw_buf_init(&s_draw_buf, s_draw_buf_1, NULL, pixel_count);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = LCD_ILI9488_H_RES;
    s_disp_drv.ver_res = LCD_ILI9488_V_RES;
    s_disp_drv.flush_cb = lvgl_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;
    lv_disp_drv_register(&s_disp_drv);

    const esp_timer_create_args_t tick_timer_args = {
        .callback = lvgl_tick_cb,
        .name = "lvgl_tick",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&tick_timer_args, &s_lvgl_tick_timer),
                        TAG, "create lvgl tick");
    return esp_timer_start_periodic(s_lvgl_tick_timer, LVGL_TICK_MS * 1000);
}

static void update_avatar_frame(uint32_t frame)
{
    static uint32_t last_frame = UINT32_MAX;
    if (frame >= ARRAY_SIZE(s_avatar_frames) || frame == last_frame) {
        return;
    }
    lv_img_set_src(s_avatar_frame_img, s_avatar_frames[frame]);
    last_frame = frame;
}

static void animation_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    static const uint16_t frame_durations_ms[AVATAR_FRAME_COUNT] = {
        420, 170, 170, 180, 180, 180, 120, 130, 170, 180, 180, 420,
    };
    static uint32_t frame;
    static uint32_t elapsed_ms;

    elapsed_ms += ANIM_TIMER_MS;
    while (elapsed_ms >= frame_durations_ms[frame]) {
        elapsed_ms -= frame_durations_ms[frame];
        frame = (frame + 1) % AVATAR_FRAME_COUNT;
    }

    update_avatar_frame(frame);
}

static void create_ui(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_remove_style_all(screen);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    s_avatar_frame_img = lv_img_create(screen);
    lv_img_set_src(s_avatar_frame_img, &avatar_frame_0);
    lv_obj_set_pos(s_avatar_frame_img, AVATAR_FRAME_X, AVATAR_FRAME_Y);

    lv_timer_create(animation_timer_cb, ANIM_TIMER_MS, NULL);
    animation_timer_cb(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 ILI9488 LVGL full-screen companion animation");
    ESP_LOGI(TAG, "LCD VCC=5V/3V3 GND=GND SCK=39 MOSI=38 MISO=40 CS=41 DC=42 RST=16 LED=2");

    ESP_ERROR_CHECK(lcd_ili9488_init());
    ESP_ERROR_CHECK(lcd_ili9488_fill_screen_rgb565(0x0000));
    ESP_ERROR_CHECK(lvgl_port_init());
    create_ui();

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(8));
    }
}
