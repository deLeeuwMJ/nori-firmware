#include "display.hpp"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "driver/gpio.h"
#include "config.hpp"

TouchDisplay::~TouchDisplay()
{
    panel_handle = NULL;
    io_handle = NULL;
}

void TouchDisplay::init()
{
    ESP_ERROR_CHECK(turnOffDisplay());
    ESP_ERROR_CHECK(installPanelIO());
    ESP_ERROR_CHECK(installPanelDriver());
    setPanelDefaults();
    ESP_ERROR_CHECK(turnOnDisplay());
}

esp_err_t TouchDisplay::turnOnDisplay()
{
    ESP_LOGI(TAG, "Turn on Display");

    auto ret = esp_lcd_panel_disp_on_off(panel_handle, true);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Turn on LCD backlight");
    return gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, CONFIG_LCD_BK_LIGHT_ON_LEVEL);
}

esp_err_t TouchDisplay::turnOffDisplay()
{
    ESP_LOGI(TAG, "Turn off LCD backlight");

    gpio_config_t bk_gpio_config = {
        .pin_bit_mask = 1ULL << CONFIG_PIN_NUM_BK_LIGHT,
        .mode = GPIO_MODE_OUTPUT
    };

    return gpio_config(&bk_gpio_config);
}

esp_err_t TouchDisplay::installPanelIO()
{
    ESP_LOGI(TAG, "Install panel IO");

    esp_lcd_panel_io_spi_config_t lcd_config = {
        .cs_gpio_num = CONFIG_PIN_NUM_LCD_CS,
        .dc_gpio_num = CONFIG_PIN_NUM_LCD_DC,
        .spi_mode = 0,
        .pclk_hz = CONFIG_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };

    return esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &lcd_config, &io_handle);
}

esp_err_t TouchDisplay::installPanelDriver()
{
    ESP_LOGI(TAG, "Install GC9A01 panel driver");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CONFIG_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };

    return esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle);
}

void TouchDisplay::setPanelDefaults()
{
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
}