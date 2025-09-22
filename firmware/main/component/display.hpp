#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"

#define LCD_H_RES               240
#define LCD_V_RES               240
#define LCD_CMD_BITS            8
#define LCD_PARAM_BITS          8
#define LCD_HOST                SPI2_HOST

#define LCD_PIXEL_CLOCK_HZ      (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL   1
#define LCD_BK_LIGHT_OFF_LEVEL  !CONFIG_LCD_BK_LIGHT_ON_LEVEL

#define LCD_GPIO_DC             8
#define LCD_GPIO_RST            14
#define LCD_GPIO_CS             9
#define LCD_GPIO_BK_LIGHT       GPIO_NUM_2

#define LOG_TAG_DISPLAY         "DISPLAY"

class TouchDisplay
{
    public:
        ~TouchDisplay();

        esp_lcd_panel_handle_t getPanelHandle()
        {
            return panel_handle;
        };

        esp_lcd_panel_io_handle_t getPanelIOHandle()
        {
            return io_handle;
        };

        void init();
    private:
        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_panel_io_handle_t io_handle = NULL;

        esp_err_t turnOnDisplay();
        esp_err_t turnOffDisplay();
        esp_err_t installPanelIO();
        esp_err_t installPanelDriver();

        void setPanelDefaults();
};

