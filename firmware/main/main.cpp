#include <stdio.h>
#include "esp_err.h"
#include "config.hpp"
#include "display.hpp"
#include "render.hpp"
#include "touch.hpp"
#include "core.hpp"

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" {
    void app_main(void) {
        ESP_ERROR_CHECK(core::init_spi_bus());
        ESP_ERROR_CHECK(core::init_i2c_bus());

        Touch* touch = new Touch();
        touch->setup();

        TouchDisplay* touchDisplay = new TouchDisplay();
        touchDisplay->init();

        Render* render = new Render();
        render->setup(*touchDisplay);
    }
}