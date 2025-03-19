#include "lvhelper.h"
#include "../helper/confighelper.h"


void setlvalignment(lv_obj_t* obj, SimUIWidgetAlign alignment, int xpos, int ypos)
{

    switch (alignment)
    {
        case SIMUI_WIDGETALIGN_TOP_LEFT:
            lv_obj_align(obj, LV_ALIGN_TOP_LEFT, xpos, ypos);
            break;
        case SIMUI_WIDGETALIGN_TOP_RIGHT:
            lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, xpos, ypos);
            break;
        case SIMUI_WIDGETALIGN_BOTTOM_LEFT:
            lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, xpos, ypos);
            break;
        case SIMUI_WIDGETALIGN_BOTTOM_RIGHT:
            lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, xpos, ypos);
            break;
        case SIMUI_WIDGETALIGN_CENTER:
        default:
            lv_obj_align(obj, LV_ALIGN_CENTER, xpos, ypos);
            break;
    }
}


void setlvfont(lv_obj_t* obj, SimUIWidget widget, lv_font_t** fonts, int numfonts)
{

    int fontid = widget.fontid;
    if(widget.customfont == false)
    {
        lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN);
    }
    else
    {
        if(numfonts >= fontid)
        {
            lv_obj_set_style_text_font(obj, fonts[fontid-1], LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN);
        }
    }
}

void setlvcolor(lv_obj_t* obj, uint32_t color)
{
    uint8_t red = (color >> 8) & 0xFF;
    uint8_t green = (color >> 16) & 0xFF;
    uint8_t blue = (color >> 24) & 0xFF;

    lv_obj_set_style_text_color(obj, lv_color_make(red, green, blue), LV_PART_MAIN|LV_STATE_DEFAULT);
}
