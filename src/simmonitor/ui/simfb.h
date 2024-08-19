#include <uv.h>

#include "../gameloop/loopdata.h"

int fbinit(FontInfo* fi, int fonts, int widgets, loop_data* l);
void fbstop();
void fbcallback(uv_timer_t* handle);
