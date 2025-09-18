#pragma once

#include "lvgl.h"

class Render
{
    public:
        void loadUserInterface(lv_display_t *disp);
    private:
        void setObjectAngle(void * obj, int32_t v);
};