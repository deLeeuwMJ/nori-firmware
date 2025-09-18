#pragma once

#define MIN(a, b) (a) > (b) ? (b) : (a)
#define MAX(a, b) (a) > (b) ? (a) : (b)

#define LCD_HOST  SPI2_HOST

#define CONFIG_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define CONFIG_LCD_BK_LIGHT_ON_LEVEL  1
#define CONFIG_LCD_BK_LIGHT_OFF_LEVEL !CONFIG_LCD_BK_LIGHT_ON_LEVEL

#define CONFIG_PIN_NUM_SCLK           10
#define CONFIG_PIN_NUM_MOSI           11
#define CONFIG_PIN_NUM_LCD_DC         8
#define CONFIG_PIN_NUM_LCD_RST        14
#define CONFIG_PIN_NUM_LCD_CS         9
#define CONFIG_PIN_NUM_BK_LIGHT       GPIO_NUM_2
#define CONFIG_PIN_RST                13

#define LCD_H_RES              240
#define LCD_V_RES              240
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define LVGL_DRAW_BUF_LINES    20 // number of display lines in each draw buffer
#define LVGL_TICK_PERIOD_MS    2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define LVGL_TASK_STACK_SIZE   (4 * 1024)
#define LVGL_TASK_PRIORITY     2