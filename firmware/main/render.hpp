#pragma once

#include "lvgl.h"
#include "display.hpp"

class Render
{
    public:
        void setup(TouchDisplay& touchDisplay);
        void loadUserInterface();
        void UpdateValue(uint8_t value);
    private:
        static constexpr const char *LVGL_TAG = "LVGL_TASK";
        const char *TAG = "Render";

        static void lvglMainLoopTask(void *arg);
        static void lvglTickTimerCallback(void *arg);
        static void lvglFlushScreen(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
        static bool lvglNotifyFlushReadyCallback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);

        lv_obj_t *textLabel = nullptr;
    };