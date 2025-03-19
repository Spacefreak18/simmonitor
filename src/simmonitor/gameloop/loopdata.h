#ifndef _LOOPDATA_H
#define _LOOPDATA_H

#include <uv.h>
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../ui/lvhelper.h"

typedef struct loop_data
{
    uv_work_t req;
    SimulatorAPI sim;
    SimulatorAPI mapapi;
    int numfonts;
    int numwidgets;
    bool frame_lock;
    bool simstate;
    bool uion;
    bool releasing;
    bool use_udp;
    SMSettings* sms;
    SimData* simdata;
    SimMap* simmap;
    SimUIWidget* simuiwidgets;
    char* templatefile;
    lv_font_t** simlvfonts;
    lv_obj_t** simlvobjs;
    struct fttinfo* ft;
    FontInfo* fi;
    char* css;
    char* js;
} loop_data;

#endif
