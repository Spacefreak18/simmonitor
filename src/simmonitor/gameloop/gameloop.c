#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <MQTTClient.h>
#include <poll.h>
#include <termios.h>
#include <hoel.h>
#include <jansson.h>
#include <uv.h>

#include "gameloop.h"
#include "loopdata.h"

#include "../db/hoeldb.h"
#include "../telemetry/telemetry.h"
#include "../telemetry/telemmonitor.h"
#include "../ui/ncursesui.h"
#include "../ui/xgui.h"
#include "../ui/simwebui.h"
#include "../ui/simfb.h"

#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../helper/dirhelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"


uv_idle_t idler;
uv_timer_t datachecktimer;
uv_timer_t datamaptimer;
uv_timer_t cursestimer;
uv_timer_t xtimer;
uv_timer_t fbtimer;
uv_timer_t telemetrytimer;

bool doui = false;
int appstate = 0;

struct MHD_Daemon* d;

void shmdatamapcallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);
void startui(UIType ui, SMSettings* sms, loop_data* l);
void startdatalogger(SMSettings* sms, loop_data* l);
void stopui(UIType ui, loop_data* l);
void stopdatalogger(SMSettings* sms, loop_data* l);

void shmdatamapcallback(uv_timer_t* handle)
{

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SMSettings* sms = f->sms;
    //appstate = 2;
    if (appstate == 2)
    {
        simdatamap(simdata, simmap, f->sim);
        if (doui == true)
        {
            slogi("looking for ui config %s", sms->uiconfig_str);
            int confignum = getuiconfigtouse(sms->uiconfig_str, simdata->car, f->sim);

            int fonts = 0;
            int widgets = 0;
            uiconfigcheck(sms->uiconfig_str, confignum, &fonts, &widgets);
            slogd("loading confignum %i, with %i widgets, and %i fonts.", confignum, fonts, widgets);
            f->numfonts = fonts;
            f->numwidgets = widgets;

            FontInfo* fi = malloc(sizeof(FontInfo) * fonts);
            SimUIWidget* simuiwidgets = malloc(sizeof(SimUIWidget) * widgets);

            uiloadconfig(sms->uiconfig_str, confignum, fi, simuiwidgets, "");


            f->simuiwidgets = simuiwidgets;
            f->fi = fi;
            doui = false;
            f->uion = true;
            slogd("starting ui");
            startui(sms->ui_type, sms, f);
            startdatalogger(sms, f);
        }
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate == 1)
    {
        f->uion = false;
        slogi("stopped mapping data, press q again to quit");
        stopdatalogger(sms, f);
        stopui(sms->ui_type, f);
        uv_timer_stop(handle);
        uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
    }
}

void datacheckcallback(uv_timer_t* handle)
{

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;

    if ( appstate == 1 )
    {
        getSim(simdata, simmap, &f->simstate, &f->sim);
    }
    if (f->simstate == true)
    {
        if ( appstate == 1 )
        {
            appstate++;
            doui = true;
            uv_timer_start(&datamaptimer, shmdatamapcallback, 2000, 1000);
            uv_timer_stop(handle);
        }
    }

    if (appstate == 0)
    {
        slogi("stopping checking for data");
        uv_timer_stop(handle);
    }
}

void stopui(UIType ui, loop_data* f)
{
    switch (ui)
    {
        case (SIMMONITOR_CLI):
        case (SIMMONITOR_CURSES):
        case (SIMMONITOR_X):
        case (SIMMONITOR_FB):
            break;
        case (SIMMONITOR_WEB):
            webuistop(d);
            free(f->css);
            break;
    }
}

void startdatalogger(SMSettings* sms, loop_data* f)
{
    if(sms->mysql == true)
    {
        telemetryinit(f->simdata, f->simmap, sms);
        uv_timer_start(&telemetrytimer, telemetrycallback, 0, 250);
    }
}

void stopdatalogger(SMSettings* sms, loop_data* f)
{
    if(sms->mysql == true)
    {
        telemetrystop(f->simdata);
    }
}

void startui(UIType ui, SMSettings* sms, loop_data* f)
{
    switch (ui)
    {
        case (SIMMONITOR_CLI):
            fprintf(stdout, "Press q to quit...\n");
            fprintf(stdout, "Press c for a useful readout of car telemetry...\n");
            fprintf(stdout, "Press s for basic sesion information...\n");
            fprintf(stdout, "Press l for basic lap / stint information...\n");
            break;
        case (SIMMONITOR_CURSES):
            curses_init();
            uv_timer_start(&cursestimer, cursescallback, 0, 16);
            break;
        case (SIMMONITOR_X):
            xinit();
            uv_timer_start(&xtimer, xcallback, 0, 16);
            break;
        case (SIMMONITOR_FB):
            fbinit(f->fi, f->numfonts, f->numwidgets, f);
            uv_timer_start(&fbtimer, fbcallback, 0, 16);
            break;
        case (SIMMONITOR_WEB):
            slurp(sms->css_file_str, &f->css, true);
            webuistart(f);
            slogi("starting microhttpd daemon on port 2300...");
            d = MHD_start_daemon (MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                                  2300, NULL, NULL, &ahc_echo, NULL, MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120, MHD_OPTION_END);
            break;
    }
}

void cb(uv_poll_t* handle, int status, int events)
{

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;

    char* line = NULL;
    size_t len = 0;
    char ch;
    scanf("%c", &ch);
    if (ch == 'q')
    {
        appstate--;
        slogi("User requested stop appstate is now %i", appstate);
        fprintf(stdout, "User requested stop appstate is now %i\n", appstate);
        fflush(stdout);
    }

    if(appstate == 2)
    {
        if(ch == 'c')
        {
            fprintf(stdout, "speed: %i rpms: %i gear: %i\n", simdata->velocity, simdata->rpms, simdata->gear);
            fflush(stdout);
        }
        if(ch == 's')
        {
            fprintf(stdout, "status: %i\n", simdata->simstatus);
            fflush(stdout);
        }
    }

    if (appstate == 1)
    {
        f->uion = false;
    }
    if (appstate == 0)
    {
        slogi("Sim Monitor is exiting...");
        uv_poll_stop(handle);
    }
}



int mainloop(SMSettings* sms)
{

    SimData* simdata = malloc(sizeof(SimData));
    SimMap* simmap = malloc(sizeof(SimMap));

    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);
    char ch;
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };

    uv_poll_t* poll = (uv_poll_t*) malloc(uv_handle_size(UV_POLL));

    loop_data* baton = (loop_data*) malloc(sizeof(loop_data));
    baton->simmap = simmap;
    baton->simdata = simdata;
    baton->sms = sms;
    baton->simstate = false;
    baton->uion = false;
    baton->sim = 0;
    baton->req.data = (void*) baton;
    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &cursestimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &xtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &fbtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &telemetrytimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) poll, (void*) baton);
    appstate = 1;
    slogd("setting initial app state");
    uv_timer_init(uv_default_loop(), &datachecktimer);
    fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);


    if (0 != uv_poll_init(uv_default_loop(), poll, 0))
    {
        return 1;
    };
    if (0 != uv_poll_start(poll, UV_READABLE, cb))
    {
        return 2;
    };


    uv_timer_init(uv_default_loop(), &datamaptimer);
    uv_timer_init(uv_default_loop(), &cursestimer);
    uv_timer_init(uv_default_loop(), &xtimer);
    uv_timer_init(uv_default_loop(), &fbtimer);
    uv_timer_init(uv_default_loop(), &telemetrytimer);


    //uv_idle_init(uv_default_loop(), &idler);
    //uv_idle_start(&idler, idle_cb);

    //uv_prepare_init(uv_default_loop(), &prep);
    //uv_prepare_start(&prep, prep_cb);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    fprintf(stdout, "\n");
    fflush(stdout);
    tcsetattr(0, TCSANOW, &canonicalmode);

    free(simdata);
    free(simmap);

    return 0;
}

