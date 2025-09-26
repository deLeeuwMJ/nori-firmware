#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "driver/gpio.h"

#include "display.hpp"
#include "../core/board.hpp"

TouchDisplay::~TouchDisplay()
{
    panel_handle = NULL;
    io_handle = NULL;
}

void TouchDisplay::init()
{
    ESP_ERROR_CHECK(installPanelIO());
    ESP_ERROR_CHECK(installPanelDriver());
    setPanelDefaults();
}

esp_err_t TouchDisplay::installPanelIO()
{
    ESP_LOGI(LOG_TAG_DISPLAY, "Install panel IO");

    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_GPIO_CS,
        .dc_gpio_num = LCD_GPIO_DC,
        .spi_mode = 0,
        .pclk_hz = (40 * 1000 * 1000),
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    return esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);
}

esp_err_t TouchDisplay::installPanelDriver()
{
    ESP_LOGI(LOG_TAG_DISPLAY, "Install GC9A01 panel driver");

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_GPIO_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };

    return esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle);
}

void TouchDisplay::setPanelDefaults()
{
    gpio_config_t bk_gpio_config = {
        .pin_bit_mask = 1ULL << LCD_GPIO_BK_LIGHT,
        .mode = GPIO_MODE_OUTPUT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    gpio_set_level(LCD_GPIO_BK_LIGHT, 1);
}