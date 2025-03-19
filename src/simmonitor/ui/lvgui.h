#include <uv.h>
#include "lvhelper.h"
#include "../helper/confighelper.h"

int lvgui(int uitype, int xres, int yres, int display, bool fullscreen, bool bordered);

void lvcallback(uv_timer_t* handle);

int lvinit(lv_obj_t** simlvobjs, lv_font_t** simlvfonts, FontInfo* fi, char* fontdir, int numfonts, SMSettings* sms);

int lvclear();
