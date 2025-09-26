#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "driver/i2c.h"

#include "../component/display.hpp"

#define LOG_TAG_HAL "HAL"

#define GPIO_SPI_SCLK           GPIO_NUM_10
#define GPIO_SPI_MOSI           GPIO_NUM_11

#define GPIO_I2C_SDA            6
#define GPIO_I2C_SCL            7
#define I2C_CLK_SPEED           400000

#define GPIO_RST                13

namespace core {
    esp_err_t init_spi_bus();
    esp_err_t init_i2c_bus();
}