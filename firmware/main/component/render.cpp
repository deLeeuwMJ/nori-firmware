
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

void Render::loadUserInterface()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    textLabel = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(textLabel, "Nori");
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 0);
}

void Render::updateValue(Events::CarEventData data)
{
    if (textLabel)
    {
        lvgl_port_lock(0);
        lv_label_set_text_fmt(textLabel, "%d km/h", std::stoi(data.value.c_str(), nullptr, 16));
        lvgl_port_unlock();
    }
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