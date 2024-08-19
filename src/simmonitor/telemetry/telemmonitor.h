#include <uv.h>

#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"


void telemetryinit(SimData* SimData, SimMap* simmap, SMSettings* sms);
void telemetrycallback(uv_timer_t* handle);
void telemetrystop(SimData* SimData);
