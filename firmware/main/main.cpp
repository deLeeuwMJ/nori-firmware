#include <stdio.h>
#include "esp_err.h"

#include "component/display.hpp"
#include "component/render.hpp"
#include "component/touch.hpp"

#include "core/bluetooth.hpp"
#include "core/board.hpp"

TouchDisplay* touchDisplay = nullptr;
Render* render = nullptr;

static void dataCallback(Events::CarEventData data)
{
    if (render)
        render->updateValue(data);
}

extern "C" void app_main(void) {
    ESP_ERROR_CHECK(core::init_spi_bus());
    ESP_ERROR_CHECK(core::init_i2c_bus());
    
    touchDisplay = new TouchDisplay();
    touchDisplay->init();

    render = new Render();
    render->setup(*touchDisplay);

    core::Bluetooth bluetooth = core::Bluetooth();
    bluetooth.setup(&dataCallback);
}