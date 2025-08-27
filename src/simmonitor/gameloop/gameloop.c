#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
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
//#include "../ui/xgui.h"
#include "../ui/simwebui.h"
//#include "../ui/simfb.h"
#include "../ui/lvgui.h"

#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../helper/dirhelper.h"
#include "../helper/iohelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"


uv_idle_t idler;
uv_timer_t datachecktimer;
uv_timer_t datamaptimer;
uv_timer_t cursestimer;
uv_timer_t xtimer;
uv_timer_t lvtimer;
uv_timer_t fbtimer;
uv_timer_t telemetrytimer;
uv_udp_t recv_socket;

bool doui = false;
int appstate = 0;

struct MHD_Daemon* d;

void shmdatamapcallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);
void startui(UIType ui, SMSettings* sms, loop_data* l, SimData* simdata);
void startdatalogger(SMSettings* sms, loop_data* l);
void stopui(UIType ui, loop_data* l);
void stopdatalogger(SMSettings* sms, loop_data* l);

void simapilib_loginfo(char* message)
{
    slogi(message);
}

void simapilib_logdebug(char* message)
{
    slogd(message);
}

void simapilib_logtrace(char* message)
{
    slog_display(SLOG_TRACE, 1, message);
}

void loopstart(SMSettings* sms, loop_data* f, SimData* simdata)
{
    doui = false;
    f->uion = true;
    slogd("starting ui");
    startui(sms->ui_type, sms, f, simdata);
    startdatalogger(sms, f);
    //free(f->fi);
}

void on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
    bzero(buf->base, suggested_size);
    slogt("udp malloc:%lu %p\n",buf->len,buf->base);
}

static void on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* rcvbuf, const struct sockaddr* addr, unsigned flags) {
    if (nread > 0) {
        slogt("udp data received");
    }

    char* a;
    a = rcvbuf->base;

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SMSettings* sms = f->sms;

    if (appstate == 2)
    {
        simdatamap(simdata, simmap, NULL, f->mapapi, false, NULL);
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        if(f->releasing == false)
        {
            f->releasing = true;
            f->uion = false;
            slogi("stopped mapping data, press q again to quit");
            stopdatalogger(sms, f);
            sleep(2);
            stopui(sms->ui_type, f);
            // free loop data
            uv_udp_recv_stop(handle);

            if(appstate > 0)
            {
                uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
    }

    slogt("udp free  :%lu %p\n",rcvbuf->len,rcvbuf->base);
    free(rcvbuf->base);
}

int startudp(int port)
{
    uv_udp_init(uv_default_loop(), &recv_socket);
    struct sockaddr_in recv_addr;
    uv_ip4_addr("0.0.0.0", port, &recv_addr);
    int err = uv_udp_bind(&recv_socket, (const struct sockaddr *) &recv_addr, UV_UDP_REUSEADDR);

    slogt("initial udp error is %i", err);
    return err;
}

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
        simdatamap(simdata, simmap, NULL, f->mapapi, false, NULL);
        if (doui == true)
        {
            loopstart(sms, f, simdata);
        }
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        if(f->releasing == false)
        {
            f->releasing = true;
            f->uion = false;
            slogi("stopped mapping data, press q again to quit");
            stopdatalogger(sms, f);
            stopui(sms->ui_type, f);
            // free loop data
            uv_timer_stop(handle);

            if(appstate > 0)
            {
                uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
    }
}


void udpstart(SMSettings* sms, loop_data* f, SimData* simdata, SimMap* simmap)
{
    if (appstate == 2)
    {
        simdatamap(simdata, simmap, NULL, f->mapapi, true, NULL);
        if (doui == true)
        {
            loopstart(sms, f, simdata);
        }
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
        SimInfo si = getSim(simdata, simmap, f->sms->force_udp_mode, startudp, false);
        //TODO: move all this to a siminfo struct in loop_data
        f->simstate = si.isSimOn;
        f->sim = si.simulatorapi;
        f->mapapi = si.mapapi;
        f->use_udp = si.SimUsesUDP;

        if(f->sms->force_udp_mode == true)
        {
            f->use_udp = true;
        }
    }
    if (f->simstate == true && simdata->simstatus >= 2)
    {
        slogt("valid sim status");
        if ( appstate == 1 )
        {
            appstate++;
            doui = true;

            if(f->use_udp == true)
            {
                slogt("starting udp receive loop");
                udpstart(f->sms, f, simdata, simmap);
                uv_udp_recv_start(&recv_socket, on_alloc, on_udp_recv);
                slogt("udp receive loop started");
            }
            else
            {
                slogt("starting data mapping");
                uv_timer_start(&datamaptimer, shmdatamapcallback, 2000, 16);
            }
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
    SMSettings* sms = f->sms;
    switch (ui)
    {

        case (SIMMONITOR_WEB):
            webuistop(d);
            free(f->simuiwidgets);
            if(sms->web_def_file == NULL)
            {
                break;
            }
            free(f->css);
            free(f->js);
            free(f->templatefile);

            break;
        case (SIMMONITOR_CLI):
        case (SIMMONITOR_CURSES):
            break;
        case (SIMMONITOR_X):
        case (SIMMONITOR_FB):
        case (SIMMONITOR_SDL):
        case (SIMMONITOR_DRM):
            free(f->fi);
            free(f->simlvobjs);
            free(f->simlvfonts);
            free(f->simuiwidgets);
            break;
        default:
            break;
    }


}

void startdatalogger(SMSettings* sms, loop_data* f)
{
    if(sms->monitor == true)
    {
        telemetryinit(f->simdata, f->simmap, sms);
        uv_timer_start(&telemetrytimer, telemetrycallback, 0, 250);
    }
}

void stopdatalogger(SMSettings* sms, loop_data* f)
{
    if(sms->monitor == true)
    {
        telemetrystop(f->simdata);
        uv_timer_stop(&telemetrytimer);
    }
}

void startui(UIType ui, SMSettings* sms, loop_data* f, SimData* simdata)
{
    switch (ui)
    {
        case (SIMMONITOR_CLI):
            fprintf(stdout, "Press q to quit...\n");
            fprintf(stdout, "Press c for a useful readout of car telemetry...\n");
            fprintf(stdout, "Press s for basic sesion information...\n");
            fprintf(stdout, "Press l for basic lap / stint information...\n");
            fprintf(stdout, "Press t for tyre telemetry...\n");
            break;
        case (SIMMONITOR_CURSES):
            curses_init();
            uv_timer_start(&cursestimer, cursescallback, 0, 16);
            break;
        case (SIMMONITOR_X):
        case (SIMMONITOR_SDL):
        case (SIMMONITOR_DRM):
        case (SIMMONITOR_FB):

            slogi("looking for ui config %s", sms->uiconfig_str);
            int confignum = getuiconfigtouse(sms->uiconfig_str, simdata->car, f->sim);

            int fonts = 0;
            int widgets = 0;
            uiconfigcheck(sms->uiconfig_str, confignum, &fonts, &widgets);
            slogd("loading confignum %i, with %i widgets, and %i fonts.", confignum, widgets, fonts);
            f->numfonts = fonts;
            f->numwidgets = widgets;

            FontInfo* fi = calloc(sizeof(FontInfo), fonts);
            SimUIWidget* simuiwidgets = calloc(sizeof(SimUIWidget), widgets);
            lv_obj_t** simlvobjs = calloc(sizeof(lv_obj_t*), 100);
            lv_font_t** simlvfonts = calloc(sizeof(lv_font_t*), 10);

            uiloadconfig(sms->uiconfig_str, confignum, fi, simuiwidgets, "/usr/share/fonts/TTF", sms);
            f->simuiwidgets = simuiwidgets;
            f->fi = fi;
            f->simlvobjs = simlvobjs;
            f->simlvfonts = simlvfonts;

            //lvgui();
            lvclear();
            lvinit(f->simlvobjs, f->simlvfonts, f->fi, "/usr/share/fonts/TTF", f->numfonts, sms);
            slogi("lv ui started");
            uv_timer_start(&lvtimer, lvcallback, 0, 16);
            break;
        case (SIMMONITOR_WEB):

            slogi("looking for ui config %s", sms->uiconfig_str);
            int webconfignum = getuiconfigtouse(sms->uiconfig_str, simdata->car, f->sim);

            uiconfigcheck(sms->uiconfig_str, webconfignum, &fonts, &widgets);
            slogd("loading confignum %i, with %i widgets, and %i fonts.", webconfignum, widgets, fonts);
            f->numfonts = fonts;
            f->numwidgets = widgets;

            SimUIWidget* webuiwidgets = calloc(sizeof(SimUIWidget), widgets);

            uiloadconfig(sms->uiconfig_str, confignum, fi, webuiwidgets, "/usr/share/fonts/TTF", sms);
            f->simuiwidgets = webuiwidgets;

            if(sms->web_def_file == NULL)
            {
                sloge("No web definition file, could not start web server ui");
                break;
            }

            char* rundir = NULL;
            char* rundir2 = NULL;
            char *bname;
            char *path2 = strdup(sms->web_def_file);
            bname = basename(path2);
            bname[strlen(bname)-4] = 0;
            asprintf(&rundir, "%s%s/", sms->cachedir_str, "run");
            create_dir(rundir);
            asprintf(&rundir2, "%s%s/%s/", sms->cachedir_str, "run", bname);
            create_dir(rundir2);
            free(path2);
            slogi("Using rundir %s extracting tarball %s", rundir2, sms->web_def_file);
            extract_tarball(sms->web_def_file, rundir2);
            char* cssfilepath;
            asprintf(&cssfilepath, "%s%s", rundir2, "simstyle.css");
            FILE* fb = fopen(cssfilepath, "r");
            if (fb == NULL)
            {
                sloge("error opening %s", cssfilepath);
                perror("Failed: ");
            }
            else
            {
                f->css = fslurp(fb);
            }
            fclose(fb);
            free(cssfilepath);

            char* jsfilepath;
            asprintf(&jsfilepath, "%s%s", rundir2, "simscript.js");
            FILE* fc = fopen(jsfilepath, "r");
            if (fc == NULL) {
                sloge("error opening %s", jsfilepath);
                perror("Failed: ");
            }
            else
            {
                f->js = fslurp(fc);
            }
            fclose(fc);
            free(jsfilepath);

            asprintf(&f->templatefile, "%s%s", rundir2, "base.tmpl");
            webuistart(f);
            slogi("starting microhttpd daemon on port %i...", sms->webport);
            d = MHD_start_daemon (MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                                  sms->webport, NULL, NULL, &ahc_echo, NULL, MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120, MHD_OPTION_END);
            break;
    }
}

void cb(uv_poll_t* handle, int status, int events)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    char ch;
    scanf("%c", &ch);
    slogt("input callback");
    if (ch == 'q')
    {
        if(f->releasing == false && doui == false)
        {
            appstate--;
            slogi("User requested stop appstate is now %i", appstate);
            fprintf(stdout, "User requested stop appstate is now %i\n", appstate);
            fflush(stdout);
        }
    }

    if(appstate == 2)
    {
        void* b = uv_handle_get_data((uv_handle_t*) handle);
        loop_data* f = (loop_data*) b;
        SimData* simdata = f->simdata;
        SimMap* simmap = f->simmap;
        if(ch == 'a')
        {
            fprintf(stdout, "speedx: %i speedy: %i speedz: %i\n", simdata->Xvelocity, simdata->Yvelocity, simdata->Zvelocity);
            fflush(stdout);
        }
        if(ch == 'c')
        {
            fprintf(stdout, "speed: %i rpms: %i gear: %i\n", simdata->velocity, simdata->rpms, simdata->gear);
            fflush(stdout);
        }
        if(ch == 's')
        {
            fprintf(stdout, "track: %s status: %i flag: %i airtemp: %f tracktemp: %f cars: %i\n", simdata->track, simdata->simstatus, simdata->courseflag, simdata->airtemp, simdata->tracktemp, simdata->numcars);
            fflush(stdout);
        }
        if(ch == 'l')
        {
            char* lastlap;
            asprintf(&lastlap, "%d:%02d:%03d", simdata->lastlap.minutes, simdata->lastlap.seconds, simdata->lastlap.fraction);
            char* bestlap;
            asprintf(&bestlap, "%d:%02d:%03d", simdata->bestlap.minutes, simdata->bestlap.seconds, simdata->bestlap.fraction);
            char* curlap;
            asprintf(&curlap, "%d:%02d:%03d", simdata->currentlap.minutes, simdata->currentlap.seconds, simdata->currentlap.fraction);
            fprintf(stdout, "lap: %i position: %i sector: %i currentlap: %s lastlap: %s bestlap: %s\n", simdata->lap, simdata->position, simdata->sectorindex, curlap, lastlap, bestlap);
            fflush(stdout);
            free(curlap);
            free(lastlap);
            free(bestlap);
        }
        if(ch == 'i')
        {
            fprintf(stdout, "driver: %s car: %s compound: %s\n", simdata->driver, simdata->car, simdata->tyrecompound);
            fflush(stdout);
        }
        if(ch == 't')
        {
            fprintf(stdout, "rps: %f\n", simdata->tyreRPS[0]);
            fflush(stdout);
        }
    }

    if (appstate == 0)
    {
        slogi("Sim Monitor is exiting...");
        uv_udp_recv_stop(&recv_socket);
        uv_timer_stop(&datachecktimer);
        uv_poll_stop(handle);
    }
}



int mainloop(SMSettings* sms)
{

    SimData* simdata = malloc(sizeof(SimData));
    SimMap* simmap = createSimMap();

    if(sms->ui_type == SIMMONITOR_X || sms->ui_type == SIMMONITOR_FB || sms->ui_type == SIMMONITOR_SDL || sms->ui_type == SIMMONITOR_DRM)
    {
        lvgui(sms->ui_type, sms->xres, sms->yres, sms->display, sms->fullscreen, sms->bordered);
    }

    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };

    uv_poll_t* poll = (uv_poll_t*) malloc(uv_handle_size(UV_POLL));

    loop_data* baton = (loop_data*) malloc(sizeof(loop_data));
    baton->simmap = simmap;
    baton->simdata = simdata;
    baton->sms = sms;
    baton->simstate = false;
    baton->uion = false;
    baton->sim = 0;
    baton->releasing = false;
    baton->req.data = (void*) baton;
    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &cursestimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &xtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &lvtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &fbtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &telemetrytimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &recv_socket, (void*) baton);
    uv_handle_set_data((uv_handle_t*) poll, (void*) baton);
    appstate = 1;
    slogd("setting initial app state");
    uv_timer_init(uv_default_loop(), &datachecktimer);
    fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    sleep(5);
    uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);

    set_simapi_log_info(simapilib_loginfo);
    set_simapi_log_debug(simapilib_logdebug);
    set_simapi_log_trace(simapilib_logtrace);

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
    uv_timer_init(uv_default_loop(), &lvtimer);
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

    free(baton);
    free(simdata);
    freesimmap(simmap, false);

    return 0;
}
