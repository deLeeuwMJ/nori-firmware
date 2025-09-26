#include "motion.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"

qmi8658_dev_t Motion::dev;
std::function<void(MotionEventData data)> Motion::onResult = nullptr;

void Motion::setup(i2c_master_bus_handle_t i2c_bus_handle, std::function<void(MotionEventData data)> callback)
{
    onResult = callback;

    qmi8658_init(&dev, i2c_bus_handle, QMI8658_ADDRESS_HIGH);

    xTaskCreate(processMotionEventTask, TASK_MOTION, 4096, NULL, 5, NULL);
}

void Motion::processMotionEventTask(void *pvParameter)
{
    // ESP_LOGI(LOG_TAG_MOTION_EVENT, "Started motion event task");
    float accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z;

    while (1) {
        qmi8658_read_accel(&dev, &accel_x, &accel_y, &accel_z);
        qmi8658_read_gyro(&dev, &gyro_x, &gyro_y, &gyro_z);

        onResult(
            MotionEventData{
            .gyro={
                .x = gyro_x,
                .y = gyro_y,
                .z = gyro_z,
            },
            .accelerometer= {
                .x = accel_x,
                .y = accel_y,
                .z = accel_z,
            },
        });

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}