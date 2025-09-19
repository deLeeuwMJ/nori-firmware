#pragma once

#include "esp_lcd_touch.h"
#include "functional"

class Touch
{
    public:
        void setup(std::function<void(uint16_t, uint16_t)> callback);
    private:
        static constexpr const char *TOUCH_TAG = "TOUCH_TASK";
        static constexpr const char *TAG = "Touch";

        static void touchCallback(esp_lcd_touch_handle_t tp);
        static void processTouchEventTask(void *pvParameter);

        static std::function<void(uint16_t, uint16_t)> onTouchCoordinates;
};