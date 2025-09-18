#include "touch.hpp"
#include "driver/gpio.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_err.h"
#include "esp_log.h"
#include "config.hpp"

// TODO MOVE TO USING MUTEX
static esp_lcd_touch_handle_t tp = NULL;
static bool ISR_TOUCH_TRIGGERED = false;

void Touch::touchCallback(esp_lcd_touch_handle_t tp)
{
    ISR_TOUCH_TRIGGERED = true;
}

void Touch::processTouchEventTask(void *pvParameter)
{
    while (1) {
        if (ISR_TOUCH_TRIGGERED) {
            esp_lcd_touch_read_data(tp);

            uint16_t touch_x[1];
            uint16_t touch_y[1];
            uint16_t touch_strength[1];
            uint8_t touch_cnt = 0;

            bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, touch_x, touch_y, touch_strength, &touch_cnt, 1);

            ESP_LOGI(TAG, "Polling, touch count: %d", touch_cnt);

            if (touch_cnt > 0) {
                ESP_LOGI(TAG, "Touch at x=%d, y=%d", touch_x[0], touch_y[0]);
            }

            ISR_TOUCH_TRIGGERED = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Poll every 10 ms
    }
}

void Touch::setup()
{
    ESP_LOGI(TAG, "Setup Touch");

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_13,
        .int_gpio_num = GPIO_NUM_5,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
        .interrupt_callback = touchCallback,
    };

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = {
       .dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, 
        .on_color_trans_done = 0,                         
        .user_ctx = 0,                                    
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
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(I2C_NUM_0, &tp_io_config, &tp_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp));

    xTaskCreate(processTouchEventTask, TOUCH_TAG, 4096, NULL, 5, NULL);
}