#pragma once

#include "functional"
#include "esp_lcd_touch.h"

#define LOG_TAG_TOUCH               "TOUCH"
#define LOG_TAG_TOUCH_EVENT         "TASK_TOUCH_EVENT"
#define TASK_TOUCH                  "TASK_TOUCH"

class Touch
{
    public:
        void setup(std::function<void(uint16_t, uint16_t)> callback);
    private:
        static void touchCallback(esp_lcd_touch_handle_t tp);
        static void processTouchEventTask(void *pvParameter);

        static std::function<void(uint16_t, uint16_t)> onTouchCoordinates;
};