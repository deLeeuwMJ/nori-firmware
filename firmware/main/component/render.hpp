#pragma once

#include "lvgl.h"

#include "display.hpp"
#include "motion.hpp"
#include "../common/events.hpp"

#define LOG_TAG_RENDER              "RENDER"
#define LABEL_TEXT                  "NORI"

#define COLOR_HEX_INDIGO            0x003a57
#define COLOR_HEX_PLUM              0xCE9BD1
#define COLOR_HEX_BLUE              0x92DCE5
#define COLOR_HEX_MAIZE             0xF7EC59

#define MAIN_THEME_COLOR            COLOR_HEX_PLUM

class Render
{
    public:
        void setup(TouchDisplay& touchDisplay);
        void updateCarValue(Events::CarEventData data);
        void updateMotionValue(MotionEventData data);
    private:
        lv_disp_t *disp_handle = nullptr;
        lv_obj_t *speedLabel = nullptr;

        float lastMotionValue = NULL;

        void loadUserInterface();
    };