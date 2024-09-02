#ifndef _LOOPDATA_H
#define _LOOPDATA_H

#include <uv.h>
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"


typedef struct loop_data
{
    uv_work_t req;
    Simulator sim;
    int numfonts;
    int numwidgets;
    bool simstate;
    bool uion;
    bool releasing;
    SMSettings* sms;
    SimData* simdata;
    SimMap* simmap;
    SimUIWidget* simuiwidgets;
    char* templatefile;
    struct fttinfo* ft;
    FontInfo* fi;
    char* css;
} loop_data;

#endif
