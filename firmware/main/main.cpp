#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "config.hpp"
#include "display.hpp"
#include "render.hpp"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_lcd_touch_cst816s.h"

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
static void touch_polling_task(void *pvParameter)
{
    esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)pvParameter;

    while (1) {
        esp_lcd_touch_read_data(tp); // read only when ISR was triggled

        uint16_t touch_x[1];
        uint16_t touch_y[1];
        uint16_t touch_strength[1];
        uint8_t touch_cnt = 0;

        bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, touch_x, touch_y, touch_strength, &touch_cnt, 1);

        ESP_LOGI(TAG, "Polling, touch count: %d", touch_cnt);

        if (touch_cnt > 0) {
            // Now you can log the coordinates
            ESP_LOGI(TAG, "Touch at x=%d, y=%d", touch_x[0], touch_y[0]);
        }
        
        // Delay for a short period to avoid
        // overwhelming the CPU and I2C bus.
        vTaskDelay(pdMS_TO_TICKS(100)); // Poll every 10 ms
    }
}

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" {
    void app_main(void) {
        printf("Hello world!\n");

        ESP_ERROR_CHECK(init_spi_bus());

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
        ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2cConfig.mode, 0, 0, 0));

        ESP_LOGI(TAG, "Setup Touch");
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = LCD_H_RES,
            .y_max = LCD_V_RES,
            .rst_gpio_num = GPIO_NUM_14,
            .flags = {
                .swap_xy = 0,
                .mirror_x = 0,
                .mirror_y = 0,
            },
        };

        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = {
            .dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS,
            .control_phase_bytes = 1,
            .dc_bit_offset = 0,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 0,
            .flags =
            {
                .dc_low_on_data = 0,
                .disable_control_phase = 1,
            }
        };

        esp_lcd_touch_handle_t tp;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(I2C_NUM_0, &tp_io_config, &tp_io_handle));
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp));

        xTaskCreate(touch_polling_task, "touch_task", 4096, tp, 5, NULL);

        // TouchDisplay* touchDisplay = new TouchDisplay();
        // touchDisplay->init();

        // Render* render = new Render();
        // render->setup(*touchDisplay);
    }
}