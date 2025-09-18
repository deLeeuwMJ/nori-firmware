#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "config.hpp"
#include "display.hpp"
#include "render.hpp"

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

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" {
    void app_main(void) {
        printf("Hello world!\n");

        ESP_ERROR_CHECK(init_spi_bus());
    
        TouchDisplay* touchDisplay = new TouchDisplay();
        touchDisplay->init();

        Render* render = new Render();
        render->setup(*touchDisplay);
    }
}