#pragma once

#include "lvgl.h"

#include "display.hpp"
#include "../fa_icons.h"
#include "../common/events.hpp"

#define LOG_TAG_LVGL                "LVGL"
#define LOG_TAG_RENDER              "RENDER"

#define TASK_NAME_LVGL              "LVGL"

#define LVGL_DRAW_BUF_LINES         20 // number of display lines in each draw buffer
#define LVGL_TICK_PERIOD_MS         2
#define LVGL_TASK_MAX_DELAY_MS      500
#define LVGL_TASK_MIN_DELAY_MS      1000 / CONFIG_FREERTOS_HZ
#define LVGL_TASK_STACK_SIZE        (4 * 1024)
#define LVGL_TASK_PRIORITY          2

#define FUEL_SYMBOL                 "\xEF\x94\xAF"
#define LABEL_TEXT                  "NORI"

#define COLOR_HEX_INDIGO            0x003a57
#define COLOR_HEX_WHITE             0xffffff
#define COLOR_HEX_PLUM              0xCE9BD1
#define COLOR_HEX_BLUE              0x92DCE5
#define COLOR_HEX_MAIZE             0xF7EC59

class Render
{
    public:
        void setup(TouchDisplay& touchDisplay);
        void updateValue(Events::CarEventData data);
    private:
        lv_disp_t *disp_handle = nullptr;
        lv_obj_t *textLabel = nullptr;

        void loadUserInterface();
    };