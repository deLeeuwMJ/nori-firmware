#include <stdio.h>
#include "esp_err.h"
#include "config.hpp"
#include "display.hpp"
#include "render.hpp"
#include "touch.hpp"
#include "core.hpp"

Touch* touch = nullptr;
TouchDisplay* touchDisplay = nullptr;
Render* render = nullptr;

void onTouch(uint16_t x, uint16_t y) {
    ESP_LOGI("MAIN", "Touch handled in main! Coordinates: (%d, %d)", x, y);
    render->UpdateIntValue();
}

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" {
    void app_main(void) {
        ESP_ERROR_CHECK(core::init_spi_bus());
        ESP_ERROR_CHECK(core::init_i2c_bus());

        touch = new Touch();
        touch->setup(onTouch);

        touchDisplay = new TouchDisplay();
        touchDisplay->init();

        render = new Render();
        render->setup(*touchDisplay);
    }
}