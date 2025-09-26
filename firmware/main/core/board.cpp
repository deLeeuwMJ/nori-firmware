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

    esp_err_t init_i2c_bus(i2c_master_bus_handle_t *bus_handle_out)
    {
        ESP_LOGI(LOG_TAG_HAL, "Initialize I2C bus");

        i2c_master_bus_config_t i2c_bus_config = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = GPIO_I2C_SDA,
            .scl_io_num = GPIO_I2C_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
        };

        i2c_bus_config.flags.enable_internal_pullup = false;
        
        return i2c_new_master_bus(&i2c_bus_config, bus_handle_out);
    }
}