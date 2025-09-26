#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "component/motion.hpp"
#include "component/display.hpp"
#include "component/render.hpp"
#include "component/touch.hpp"

#include "core/bluetooth.hpp"
#include "core/board.hpp"

TouchDisplay* touchDisplay = nullptr;
Render* render = nullptr;

// static void peripheralCallback(Events::CarEventData data)
// {
//     if (render)
//         render->updateValue(data);
// }

static void motionCallback(MotionEventData data)
{
    ESP_LOGI("QMI", "Accel [g]: X=%.2f, Y=%.2f, Z=%.2f | Gyro [dps]: X=%.2f, Y=%.2f, Z=%.2f", 
        data.accelerometer.x, data.accelerometer.y, data.accelerometer.z,
        data.gyro.x, data.gyro.y, data.gyro.z
    );
}

extern "C" void app_main(void) {
    i2c_master_bus_handle_t i2c_bus_handle;

    ESP_ERROR_CHECK(core::init_spi_bus());
    ESP_ERROR_CHECK(core::init_i2c_bus(&i2c_bus_handle));

    Motion* motion = new Motion();
    motion->setup(i2c_bus_handle, motionCallback);
    
    touchDisplay = new TouchDisplay();
    touchDisplay->init();

    render = new Render();
    render->setup(*touchDisplay);

    // core::Bluetooth bluetooth = core::Bluetooth();
    // bluetooth.setup(&peripheralCallback);
}