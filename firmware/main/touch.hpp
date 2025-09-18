#pragma once

#include "esp_lcd_touch.h"

class Touch
{
    public:
        void setup();
    private:
        static constexpr const char *TOUCH_TAG = "TOUCH_TASK";
        static constexpr const char *TAG = "Touch";

        static void touchCallback(esp_lcd_touch_handle_t tp);
        static void processTouchEventTask(void *pvParameter);
};