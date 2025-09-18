#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "driver/i2c.h"
#include "config.hpp"
#include "display.hpp"
#include "render.hpp"
#include "touch.hpp"

static const char *TAG = "main";

esp_err_t init_spi_bus()
{
    ESP_LOGI(TAG, "Initialize SPI bus");

    spi_bus_config_t buscfg = {
        .mosi_io_num = CONFIG_PIN_NUM_MOSI,
        .miso_io_num = 12,
                .sclk_io_num = CONFIG_PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };

    return spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
}

esp_err_t init_i2c_bus()
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2cConfig = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 6,
        .scl_io_num = 7,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
    };

    i2cConfig.master.clk_speed = 400000;

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2cConfig));
    return i2c_driver_install(I2C_NUM_0, i2cConfig.mode, 0, 0, 0);
}

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" {
    void app_main(void) {
        ESP_ERROR_CHECK(init_spi_bus());
        ESP_ERROR_CHECK(init_i2c_bus());

        Touch* touch = new Touch();
        touch->setup();

        TouchDisplay* touchDisplay = new TouchDisplay();
        touchDisplay->init();

        Render* render = new Render();
        render->setup(*touchDisplay);
    }
}