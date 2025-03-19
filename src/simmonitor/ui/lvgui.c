#include <SDL2/SDL_video.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <uv.h>

#include "lvhelper.h"

#include "../gameloop/loopdata.h"
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"

#include "lvgl/lvgl.h"
#include "lvgl/src/font/lv_font.h"

//lv_obj_t* label;

static void configure_simulator(int argc, char **argv);

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ? : dflt;
}


lv_display_t* lv_linux_fb_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();


    lv_linux_fbdev_set_file(disp, device);

    return disp;
}

lv_display_t* lv_linux_drm_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t * disp = lv_linux_drm_create();


    lv_linux_drm_set_file(disp, device, -1);

    return disp;
}


lv_display_t* lv_linux_sdl_disp_init(int xres, int yres, uint8_t flags, uint8_t displaynum)
{
    return lv_sdl_window_create(xres, yres, flags, displaynum);
}


lv_display_t* lv_linux_x_disp_init(int xres, int yres)
{
    return lv_x11_window_create("SimMonitor", xres, yres);
}

void lv_linux_disp_init(int uitype, int xres, int yres, int displaynum, bool fullscreen, bool bordered)
{
    switch (uitype)
    {
        case SIMMONITOR_X:
            lv_linux_x_disp_init(xres, yres);
            break;
        case SIMMONITOR_FB:
            lv_linux_fb_disp_init();
            break;
        case SIMMONITOR_DRM:
            lv_linux_drm_disp_init();
            break;
        case SIMMONITOR_SDL:
        default:
            slogd("setting SDL display to %i by %i, fullscreen: %i borderless %i", xres, yres, fullscreen, bordered);
            uint8_t flags = 0;
            if(fullscreen == true)
            {
                flags |= SDL_WINDOW_FULLSCREEN;
            }
            if(bordered == false)
            {
                flags |= SDL_WINDOW_BORDERLESS;
            }
            lv_display_t* disp = lv_linux_sdl_disp_init(xres, yres, flags, displaynum);
            //lv_sdl_window_set_zoom(disp, 255);
            //lv_sdl_window_set_fullscreen(disp, fullscreen);
            //lv_sdl_window_set_bordered(disp, borderless);
            break;
    }

}


void lvcallback(uv_timer_t* handle)
{

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SMSettings* sms = f->sms;
    SimUIWidget* simuiwidgets = f->simuiwidgets;


    if (f->uion == false)
    {
        uv_timer_stop(handle);
        slogi("clearing lvgl screen");
        sleep(1);
        lv_obj_clean(lv_scr_act());

        lv_obj_t* label = (lv_obj_t*)lv_label_create(lv_screen_active());
        lv_obj_set_style_text_color(label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_label_set_text(label, "Searching for SimData...");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    }
    else
    {
        for (int j = 0; j < f->numwidgets; j++)
        {

            if(f->simlvobjs[j] == NULL)
            {
                f->simlvobjs[j] = (lv_obj_t*)lv_label_create(lv_screen_active());
                setlvalignment(f->simlvobjs[j], simuiwidgets[j].alignment, simuiwidgets[j].xpos, simuiwidgets[j].ypos);
                setlvfont(f->simlvobjs[j], simuiwidgets[j], f->simlvfonts, f->numfonts);
                setlvcolor(f->simlvobjs[j], simuiwidgets[j].rgb);
            }

            //slogd("widget has xpos: %i ypos: %i", simuiwidgets[j].xpos, simuiwidgets[j].ypos);
            switch (simuiwidgets[j].uiwidgetsubtype)
            {

                case SIMUI_TEXTWIDGET_GEAR:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%s", simdata->gearc);
                    break;
                case SIMUI_TEXTWIDGET_RPMS:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%i", simdata->rpms);
                    break;
                case SIMUI_TEXTWIDGET_POSITION:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%i", simdata->position);
                    break;
                case SIMUI_TEXTWIDGET_NUMCARS:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%i", simdata->numcars);
                    break;
                case SIMUI_TEXTWIDGET_LAP:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%i", simdata->lap);
                    break;
                case SIMUI_TEXTWIDGET_LASTLAP:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%d:%02d:%03d", simdata->lastlap.minutes, simdata->lastlap.seconds, simdata->lastlap.fraction);
                    break;
                case SIMUI_TEXTWIDGET_BESTLAP:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%d:%02d:%03d", simdata->bestlap.minutes, simdata->bestlap.seconds, simdata->bestlap.fraction);
                    break;
                case SIMUI_TEXTWIDGET_FUELREMAINING:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%f", simdata->fuel);
                    break;
                case SIMUI_TEXTWIDGET_BRAKEBIAS:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%f", simdata->brakebias);
                case SIMUI_TEXTWIDGET_LAPS:
                    lv_label_set_text_fmt(f->simlvobjs[j], "%i", simdata->numlaps);
                    break;
                case SIMUI_TEXTWIDGET_STATICTEXT:
                    char* tempstr;
                    tempstr = simuiwidgets[j].text;

                        lv_label_set_text_fmt(f->simlvobjs[j], "%s", tempstr);
                    break;
            }
        }


    }

    lv_task_handler();
}

int lvclear()
{
    lv_obj_clean(lv_scr_act());

    lv_task_handler();
    return 0;
}

int lvinit(lv_obj_t** simlvobjs, lv_font_t** simlvfonts, FontInfo* fi, char* fontdir, int numfonts, SMSettings* sms)
{
    char* temp;

    for (int j = 0; j < numfonts; j++)
    {
        slogd("Adding font %s of size %i", fi[j].name, fi[j].size);
        asprintf(&temp, "%s%s", "A:", fi[j].name);
        lv_font_t* f = lv_tiny_ttf_create_file(temp, fi[j].size);
        simlvfonts[j] = f;
        free(temp);
    }
}


int lvgui(int uitype, int xres, int yres, int display, bool fullscreen, bool bordered)
{

    //configure_simulator(argc, argv);

    /* Initialize LVGL. */
    lv_init();

    /* Initialize the configured backend SDL2, FBDEV, libDRM or wayland */
    lv_linux_disp_init(uitype, xres, yres, display, fullscreen, bordered);

    lv_obj_clean(lv_scr_act());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_label_set_text(label, "Searching for SimData...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    /*Create a Demo*/
    //lv_demo_widgets();
    //lv_demo_widgets_start_slideshow();

    //lv_linux_run_loop();

    lv_task_handler();

    return 0;
}
