#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"

#define LCD_H_RES               240
#define LCD_V_RES               240
#define LCD_HOST                SPI2_HOST

#define LCD_BK_LIGHT_ON_LEVEL   1
#define LCD_BK_LIGHT_OFF_LEVEL  !LCD_BK_LIGHT_ON_LEVEL

#define LCD_GPIO_DC             GPIO_NUM_8
#define LCD_GPIO_RST            GPIO_NUM_14
#define LCD_GPIO_CS             GPIO_NUM_9
#define LCD_GPIO_BK_LIGHT       GPIO_NUM_2

#define LOG_TAG_DISPLAY         "DISPLAY"

class TouchDisplay
{
    public:
        ~TouchDisplay();

        esp_lcd_panel_handle_t getPanelHandle()
        {
            return this->panel_handle;
        };

        esp_lcd_panel_io_handle_t getPanelIOHandle()
        {
            return this->io_handle;
        };

        void init();
    private:
        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_panel_io_handle_t io_handle = NULL;

        esp_err_t installPanelIO();
        esp_err_t installPanelDriver();

        void setPanelDefaults();
};

