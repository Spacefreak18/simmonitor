#ifndef _LVHELPER_H
#define _LVHELPER_H


#include "lvgl/lvgl.h"
#include "../helper/confighelper.h"
#include "../slog/src/slog.h"

void setlvalignment(lv_obj_t* obj, SimUIWidgetAlign alignment, int xpos, int ypos);

void setlvfont(lv_obj_t* obj, SimUIWidget widget, lv_font_t** fonts, int numfonts);

void setlvcolor(lv_obj_t* obj, uint32_t color);

#endif
