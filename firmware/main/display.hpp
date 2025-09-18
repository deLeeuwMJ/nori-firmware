#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"

class Display
{
    public:
        void init();
};

class TouchDisplay: public Display
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
        const char *TAG = "TouchDisplay";

        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_panel_io_handle_t io_handle = NULL;

        esp_err_t turnOnDisplay();
        esp_err_t turnOffDisplay();
        esp_err_t installPanelIO();
        esp_err_t installPanelDriver();

        void setPanelDefaults();
};

