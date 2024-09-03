#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <uv.h>

#include "../gameloop/loopdata.h"
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"

#include "../fbgfx/src/fbgfx.h"
#include "../fbgfx/src/fbbuf.h"
#include "../fbgfx/src/render16.h"
#include "../fbgfx/src/font16.h"
#include "../fbgfx/src/render.h"
#include "../fbgfx/src/ttfont.h"

size_t pixels_len, rowbytes;
unsigned char* pixels;
unsigned xres, yres;

struct fttinfo
{
    FT_Face ft_face;
    FT_Library ft_library;
};

gfx_color_t pal16[18];


/******************************************************************************/

#define MIN(a, b) ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })
#define MAX(a, b) ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

/******************************************************************************/

void fbcallback(uv_timer_t* handle)
//int clilooper(FontInfo* fi, int fonts, SimlcdUIWidget* simlcdwidgets, int widgets, SimData* simdata, SimMap* simmap, int sim)
{

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SMSettings* sms = f->sms;
    SimUIWidget* simuiwidgets = f->simuiwidgets;
    char* err = NULL;
    struct fttinfo* ft = f->ft;
    FontInfo* fi = f->fi;

    //time_t now;
    struct tm tm;
    int ox, oy, cx, cy;
    int r;
    int i;
    //char date[40];

    ox = xres / 2;
    oy = yres / 2;

    r = MIN(xres, yres) * 2 / 3 / 2;

    gfx_clear(pixels, pixels_len);

    //char gear[2];
    //gear[0] = simdata->gearc[0];
    //gear[1] = '\0';
    //UTF32 gear2[2] = {gear[0], 0};

    char numlaps[4];
    char numcars[4];
    char brakebias[6];
    char pos[4];
    char fuel[8];
    char lap[4];
    char rpm[6];
    char lastlap[9];
    char bestlap[9];

    snprintf(rpm, 6, "%i", simdata->rpms);
    snprintf(numlaps, 4, "%03d", simdata->numlaps);
    snprintf(numcars, 4, "%03d", simdata->numcars);
    snprintf(lap, 4, "%03d", simdata->lap);
    snprintf(pos, 4, "%03d", simdata->position);
    snprintf(fuel, 8, "%f", simdata->fuel);
    //snprintf(brakebias, 6, "%f", simdata->brakebias);
    snprintf(bestlap, 9, "%d:%02d:%03d\n", simdata->bestlap.minutes, simdata->bestlap.seconds, simdata->bestlap.fraction);
    snprintf(lastlap, 9, "%d:%02d:%03d\n", simdata->lastlap.minutes, simdata->lastlap.seconds, simdata->lastlap.fraction);

    numlaps[3] = '\0';
    numcars[3] = '\0';
    pos[3] = '\0';
    lap[3] = '\0';
    fuel[7] = '\0';
    brakebias[5] = '\0';
    rpm[5] = '\0';
    lastlap[8] = '\0';
    bestlap[8] = '\0';

    for (int j = 0; j < f->numwidgets; j++)
    {

        char* tempstr;
        bool draw = false;
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_GEAR)
        {
            tempstr = simdata->gearc;
            //asprintf(&tempstr, "%s", gear);
            //tempstr = gear;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_RPMS)
        {
            tempstr = rpm;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_POSITION)
        {
            tempstr = pos;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_NUMCARS)
        {
            tempstr = numcars;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_LAP)
        {
            tempstr = lap;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_LASTLAP)
        {
            tempstr = lastlap;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_BESTLAP)
        {
            tempstr = bestlap;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_FUELREMAINING)
        {
            tempstr = fuel;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_BRAKEBIAS)
        {
            tempstr = brakebias;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_LAPS)
        {
            tempstr = numlaps;
            draw = true;
        }
        if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_STATICTEXT)
        {
            tempstr = simuiwidgets[j].text;
            draw = true;
        }
        if (draw == true)
        {
            draw_string_on_fb(
                ft[simuiwidgets[j].fontid].ft_face, ft[simuiwidgets[j].fontid].ft_library, pixels,
                simuiwidgets[j].xpos, simuiwidgets[j].ypos,
                gfx_rgb(simuiwidgets[j].red,simuiwidgets[j].green,simuiwidgets[j].blue), tempstr);
        }
    }


    gfx_swapbuffers();

    if (f->uion == false)
    {
        uv_timer_stop(handle);
    }

}

int fbstop(int numfonts, struct fttinfo* ft)
{

    for (int j = 0; j < numfonts; j++)
    {
        done_ft(ft[j].ft_library);
    }
    gfx_clear(pixels, pixels_len);
    gfx_swapbuffers();

    gfx_close();
}

int fbinit(FontInfo* fi, int fonts, int widgets, loop_data* l)
{
    char* err = NULL;
    struct fttinfo* ft = malloc(sizeof(struct fttinfo) * fonts);
    for (int j = 0; j < fonts; j++)
    {
        init_ft(fi[j].name, &ft[j].ft_face, &ft[j].ft_library, fi[j].size, &err);
    }

    if (gfx_open(&xres, &yres, &rowbytes))
    {
        return 1;
    }


    if (gfx_setbpp(16, &rowbytes))
    {
        return 1;
    }
    l->ft = ft;

    draw_init(16);

    slogt("fb draw init complete...");
    pixels = gfx_buffer(&pixels_len);
    if (!pixels)
    {
        slogf("You have no pixels");
        return 1;
    }

    gfx_clear(pixels, pixels_len);

    return 0;

    slogt("fb init complete...");
}
