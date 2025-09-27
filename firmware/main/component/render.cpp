
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
#include "misc/lv_timer.h"

#define EYE_WIDTH           40
#define EYE_HEIGHT          60
#define EYE_BORDER_WIDTH    8
#define SCLERA_RADIUS       45

#define MIN_IDLE_TIME_MS 5000 // Minimum time between blinks
#define MAX_IDLE_TIME_MS 15000 // Maximum time between blinks

const lv_coord_t BLINK_HEIGHT = 40;
const uint32_t BLINK_DURATION = 500;

static lv_obj_t *eye_sclera_left;
static lv_obj_t *eye_sclera_right;

static lv_coord_t initial_eye_y_pos_left = 60;
static lv_coord_t initial_eye_y_pos_right = 60;

#define DIZZY_MOVE_RANGE_Y          15
#define DIZZY_ANIM_DURATION_MS      150
#define DIZZY_TRIGGER_TRESHOLD      75

static lv_timer_t *idleTimer = nullptr;
static bool dizzyTriggerd = false;

static void trigger_dizzy_animation(lv_obj_t *obj) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    
    // Use lv_obj_set_y for the executive callback
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y); 
    
    // Get the object's current Y position (which should be its initial position)
    lv_coord_t start_y = lv_obj_get_y(obj);
    
    // Go from initial position to up/down
    lv_anim_set_values(&a, start_y, start_y + DIZZY_MOVE_RANGE_Y);
    
    // Set a very short time for rapid movement
    lv_anim_set_time(&a, DIZZY_ANIM_DURATION_MS);
    lv_anim_set_delay(&a, 0);
    
    lv_anim_set_playback_time(&a, DIZZY_ANIM_DURATION_MS); 
    lv_anim_set_repeat_count(&a, 3); // Repeat indefinitely until stopped
    
    lv_anim_start(&a);
}


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

    return sclera;
}

static void trigger_blink_animation(lv_obj_t *obj) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    
    // Use lv_obj_set_height for the executive callback
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height); 
    
    // BLINK state: Go from full height to half height
    lv_anim_set_values(&a, EYE_HEIGHT, BLINK_HEIGHT);
    lv_anim_set_time(&a, BLINK_DURATION);
    lv_anim_set_delay(&a, 0);
    
    // Set a playback time to immediately reverse the animation (half-closed to open)
    lv_anim_set_playback_time(&a, BLINK_DURATION); 
    
    // Ensure it only runs once (no repeat)
    lv_anim_set_repeat_count(&a, 0);
    
    lv_anim_start(&a);
}

static void idle_state_manager_cb(lv_timer_t *timer) {
    if (dizzyTriggerd)
    {
        trigger_dizzy_animation(eye_sclera_left);
        trigger_dizzy_animation(eye_sclera_right);
        dizzyTriggerd = false;

        lv_timer_set_period(timer, 1000);
    }
    else 
    {
        lv_obj_set_y(eye_sclera_left, initial_eye_y_pos_left);
        lv_obj_set_y(eye_sclera_right, initial_eye_y_pos_right);

        uint32_t roll = esp_random() % 100;
        if (roll < 75) {
            trigger_blink_animation(eye_sclera_left);
            trigger_blink_animation(eye_sclera_right);
        } else {
            trigger_blink_animation(eye_sclera_right);
        }

        uint32_t random_delay = (esp_random() % (MAX_IDLE_TIME_MS - MIN_IDLE_TIME_MS + 1)) + MIN_IDLE_TIME_MS;
        lv_timer_set_period(timer, random_delay);
    }
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

    // Create the eyes and save the references
    eye_sclera_left = create_eye(lv_screen_active(), 70, 60);
    eye_sclera_right = create_eye(lv_screen_active(), 130, 60);

    idleTimer = lv_timer_create(idle_state_manager_cb, 1000, NULL);
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
    float newMotionValue = data.accelerometer.y;
    
    bool tresholdMet = false;
    if (lastMotionValue != NULL)
        tresholdMet = std::abs(newMotionValue - lastMotionValue) > DIZZY_TRIGGER_TRESHOLD;

    if (lastMotionValue == NULL || tresholdMet)
        lastMotionValue = newMotionValue;

    if (idleTimer && tresholdMet)
    {
        dizzyTriggerd = true;
        lv_timer_ready(idleTimer);
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