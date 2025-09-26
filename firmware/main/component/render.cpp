
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"
#include "unistd.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_lvgl_port.h"
#include "render.hpp"

#define EYE_WIDTH           40
#define EYE_HEIGHT          60
#define EYE_BORDER_WIDTH    8
#define SCLERA_RADIUS       45

static void create_eye(lv_obj_t *parent, lv_coord_t x_pos, lv_coord_t y_pos) {
    lv_obj_t *sclera = lv_obj_create(parent);
    lv_obj_set_size(sclera, EYE_WIDTH, EYE_HEIGHT);
    lv_obj_set_pos(sclera, x_pos, y_pos);
    lv_obj_set_style_radius(sclera, SCLERA_RADIUS, 0);
    lv_obj_set_style_bg_color(sclera, lv_color_hex(0x003a57), 0);
    lv_obj_set_style_border_width(sclera, EYE_BORDER_WIDTH, 0);
    lv_obj_set_style_border_color(sclera, lv_color_white(), 0);
}


void Render::loadUserInterface()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    create_eye(lv_screen_active(), 70, 90);
    create_eye(lv_screen_active(), 130, 90);
}

void Render::updateValue(Events::CarEventData data)
{
    // if (textLabel)
    // {
    //     lvgl_port_lock(0);
    //     lv_label_set_text_fmt(textLabel, "%d km/h", std::stoi(data.value.c_str(), nullptr, 16));
    //     lvgl_port_unlock();
    // }
}

void Render::setup(TouchDisplay& touchDisplay)
{
    ESP_LOGI(LOG_TAG_RENDER, "Initialize LVGL library");

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = touchDisplay.getPanelIOHandle(),
        .panel_handle = touchDisplay.getPanelHandle(),
        .buffer_size = LCD_H_RES * 40,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = false,
        },
        .flags = {
            .swap_bytes = true,
        }
    };
    disp_handle = lvgl_port_add_disp(&disp_cfg);

    lvgl_port_lock(0);
    loadUserInterface();
    lvgl_port_unlock();
}