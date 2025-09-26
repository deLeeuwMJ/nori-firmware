#include "board.hpp"

namespace core
{
    esp_err_t init_spi_bus()
    {
        ESP_LOGI(LOG_TAG_HAL, "Initialize SPI bus");

        spi_bus_config_t buscfg = {
            .mosi_io_num = GPIO_SPI_MOSI,
            .miso_io_num = -1,
            .sclk_io_num = GPIO_SPI_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
        };

        return spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    }

    esp_err_t init_i2c_bus()
    {
        ESP_LOGI(LOG_TAG_HAL, "Initialize I2C bus");

        i2c_config_t i2cConfig = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = GPIO_I2C_SDA,
            .scl_io_num = GPIO_I2C_SCL,
            .sda_pullup_en = GPIO_PULLUP_DISABLE,
            .scl_pullup_en = GPIO_PULLUP_DISABLE,
        };
        i2cConfig.master.clk_speed = I2C_CLK_SPEED;

        ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2cConfig));
        return i2c_driver_install(I2C_NUM_0, i2cConfig.mode, 0, 0, 0);
    }
}