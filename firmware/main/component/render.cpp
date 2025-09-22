
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"
#include "unistd.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "render.hpp"

Render::~Render()
{
    lvgl_api_mutex = nullptr;
    fa_icon_style = nullptr;
    textLabel = nullptr;
}

void Render::loadUserInterface()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(COLOR_HEX_INDIGO), LV_PART_MAIN);
    
    lv_obj_t *logoLabel = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(logoLabel, LABEL_TEXT);
    lv_obj_set_style_text_font(logoLabel, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(logoLabel, lv_color_hex(COLOR_HEX_BLUE), LV_PART_MAIN);
    lv_obj_align(logoLabel, LV_ALIGN_CENTER, 0, -25);

    // lv_obj_t icon_fuel = lv_label_create(lv_screen_active());
    // lv_label_set_text(icon_fuel, FUEL_SYMBOL);
    // lv_obj_add_style(icon_fuel, &fa_icon_style, LV_PART_MAIN);
    // lv_obj_align(icon_fuel, LV_ALIGN_BOTTOM_MID, 0, -20);

    textLabel = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(textLabel, "-");
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(textLabel, lv_color_hex(COLOR_HEX_WHITE), LV_PART_MAIN);
    lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 5);
}

void Render::updateValue(Events::CarEventData data)
{
    if (textLabel)
        lv_label_set_text_fmt(textLabel, "%u km/h", (unsigned int)data.value);
}

void Render::setup(TouchDisplay& touchDisplay)
{
    ESP_LOGI(LOG_TAG_RENDER, "Initialize LVGL library");
    lvgl_api_mutex = xSemaphoreCreateMutex();

    lv_init();
    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
    size_t draw_buffer_sz = LCD_H_RES * LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t);

    void *buf1 = spi_bus_dma_memory_alloc(LCD_HOST, draw_buffer_sz, 0);
    assert(buf1);
    void *buf2 = spi_bus_dma_memory_alloc(LCD_HOST, draw_buffer_sz, 0);
    assert(buf2);

    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(display, touchDisplay.getPanelHandle());
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, lvglFlushScreen);

    ESP_LOGI(LOG_TAG_RENDER, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvglTickTimerCallback,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(LOG_TAG_RENDER, "Register io panel event callback for LVGL flush ready notification");
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = &lvglNotifyFlushReadyCallback,
    };

    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(touchDisplay.getPanelIOHandle(), &cbs, display));

    xTaskCreate(lvglMainLoopTask, TASK_NAME_LVGL, LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);

    // FontAwesome Icon
    lv_style_init(fa_icon_style);
    lv_style_set_text_font(fa_icon_style, &fa_icons); 
    lv_style_set_text_color(fa_icon_style, lv_color_hex(COLOR_HEX_MAIZE));

    xSemaphoreTake(lvgl_api_mutex, portMAX_DELAY);
    loadUserInterface();
    xSemaphoreGive(lvgl_api_mutex);
}

void Render::lvglMainLoopTask(void *arg)
{
    ESP_LOGI(LOG_TAG_LVGL, "Starting LVGL task");
    uint32_t time_till_next_ms = 0;

    while (1) {
        xSemaphoreTake(lvgl_api_mutex, portMAX_DELAY);
        time_till_next_ms = lv_timer_handler();
        xSemaphoreGive(lvgl_api_mutex);

        vTaskDelay(pdMS_TO_TICKS(time_till_next_ms));
    }
}

void Render::lvglTickTimerCallback(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void Render::lvglFlushScreen(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    lv_draw_sw_rgb565_swap(px_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}

bool Render::lvglNotifyFlushReadyCallback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}