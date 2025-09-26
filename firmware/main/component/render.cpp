
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"
#include "unistd.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_lvgl_port.h"
#include "render.hpp"
#include "esp_random.h"
#include "time.h"

#define EYE_WIDTH           40
#define EYE_HEIGHT          60
#define EYE_BORDER_WIDTH    8
#define SCLERA_RADIUS       45

#define MIN_IDLE_TIME_MS 5000 // Minimum time between blinks
#define MAX_IDLE_TIME_MS 15000 // Maximum time between blinks

const lv_coord_t MAX_HEIGHT = EYE_HEIGHT; 
const lv_coord_t HALF_HEIGHT = EYE_HEIGHT / 1.5;
const uint32_t BLINK_DURATION = 500;

static lv_obj_t *eye_sclera_left;
static lv_obj_t *eye_sclera_right;

static char motion_buffer[32];

static lv_obj_t *create_eye(lv_obj_t *parent, lv_coord_t x_pos, lv_coord_t y_pos) {
    lv_obj_t *sclera = lv_obj_create(parent);
    
    // Set initial size and position
    lv_obj_set_size(sclera, EYE_WIDTH, EYE_HEIGHT); 
    lv_obj_set_pos(sclera, x_pos, y_pos);
    
    // Set styles
    lv_obj_set_style_radius(sclera, SCLERA_RADIUS, 0);
    lv_obj_set_style_bg_color(sclera, lv_color_hex(MAIN_THEME_COLOR), 0);
    lv_obj_set_style_border_width(sclera, EYE_BORDER_WIDTH, 0);
    lv_obj_set_style_border_color(sclera, lv_color_white(), 0);

    return sclera; // Return the created object
}

static void trigger_blink_animation(lv_obj_t *obj) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    
    // Use lv_obj_set_height for the executive callback
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height); 
    
    // BLINK state: Go from full height to half height
    lv_anim_set_values(&a, MAX_HEIGHT, HALF_HEIGHT);
    lv_anim_set_time(&a, BLINK_DURATION);
    lv_anim_set_delay(&a, 0);
    
    // Set a playback time to immediately reverse the animation (half-closed to open)
    lv_anim_set_playback_time(&a, BLINK_DURATION); 
    
    // Ensure it only runs once (no repeat)
    lv_anim_set_repeat_count(&a, 0);
    
    lv_anim_start(&a);
}

static void idle_state_manager_cb(lv_timer_t *timer) {
    // 1. Trigger the BLINK state for both eyes simultaneously
    trigger_blink_animation(eye_sclera_left);
    trigger_blink_animation(eye_sclera_right);

    // 2. Calculate the next random delay (New IDLE period)
    uint32_t random_delay = (esp_random() % (MAX_IDLE_TIME_MS - MIN_IDLE_TIME_MS + 1)) + MIN_IDLE_TIME_MS;
    
    // 3. Reset the timer period for the next IDLE state
    lv_timer_set_period(timer, random_delay);
}

void Render::loadUserInterface()
{
    // Set the screen background color
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(MAIN_THEME_COLOR), LV_PART_MAIN);

    // Labels
    speedLabel = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(speedLabel, LABEL_TEXT);
    lv_obj_set_style_text_font(speedLabel, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(speedLabel, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(speedLabel, LV_ALIGN_CENTER, 0, 35);

    motionLabel = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(motionLabel, "-");
    lv_obj_set_style_text_font(motionLabel, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(motionLabel, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(motionLabel, LV_ALIGN_CENTER, 0, 65);

    // Create the eyes and save the references
    eye_sclera_left = create_eye(lv_screen_active(), 70, 60);
    eye_sclera_right = create_eye(lv_screen_active(), 130, 60);

    // --- Start the IDLE State Timer ---
    
    // Calculate the initial random delay for the first blink
    uint32_t initial_delay = (rand() % (MAX_IDLE_TIME_MS - MIN_IDLE_TIME_MS + 1)) + MIN_IDLE_TIME_MS;

    // Create and start the timer
    // The timer runs the `idle_state_manager_cb` after the `initial_delay`
    lv_timer_create(idle_state_manager_cb, initial_delay, NULL);
}

void Render::updateCarValue(Events::CarEventData data)
{
    if (speedLabel)
    {
        lvgl_port_lock(0);
        lv_label_set_text_fmt(speedLabel, "%d km/h", std::stoi(data.value.c_str(), nullptr, 16));
        lvgl_port_unlock();
    }
}

void Render::updateMotionValue(MotionEventData data)
{
    if (motionLabel)
    {
        lvgl_port_lock(0);
        sprintf(motion_buffer, "%.2f", data.accelerometer.y);
        lv_label_set_text_fmt(motionLabel, "Y: %s", motion_buffer);
        lvgl_port_unlock();
    }
}

void Render::setup(TouchDisplay& touchDisplay)
{
    ESP_LOGI(LOG_TAG_RENDER, "Initialize LVGL library");

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = touchDisplay.getPanelIOHandle(),
        .panel_handle = touchDisplay.getPanelHandle(),
        .buffer_size = LCD_H_RES * 40,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = false,
        },
        .flags = {
            .swap_bytes = true,
        }
    };
    disp_handle = lvgl_port_add_disp(&disp_cfg);

    lvgl_port_lock(0);
    loadUserInterface();
    lvgl_port_unlock();
}