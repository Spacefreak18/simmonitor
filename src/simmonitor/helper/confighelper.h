#ifndef _CONFIGHELPER_H
#define _CONFIGHELPER_H

#include <stdbool.h>
#include <stdint.h>

#include <libconfig.h>

#include "parameters.h"

typedef enum
{
    SIMMONITOR_CLI    = 0,
    SIMMONITOR_X      = 1,
    SIMMONITOR_FB     = 2,
    SIMMONITOR_WEB    = 3,
    SIMMONITOR_CURSES = 4
}
UIType;

typedef struct
{
    ProgramAction program_action;
    UIType ui_type;
    int fonts_length;
    int widgets_length;
    char* uiconfig_str;
    char* dbconfig_str;
    char* css_file_str;
    char* datadir_str;
    char* log_filename_str;
    char* log_dirname_str;
    char* gnuplot_file_str;
    char* gnuplot_bin_str;

    int gnuplotfound;

    bool  mysql;
    int dberr;

    char* db_user;
    char* db_serv;
    char* db_dbnm;
    char* db_pass;
    char* db_conn;
}
SMSettings;

typedef struct
{
    int size;
    char* name;
}
FontInfo;

typedef enum
{
    SIMUI_WIDGET_TEXT       = 0,
    SIMUI_WIDGET_BOX        = 1,
}
SimUIWidgetType;

typedef enum
{
    SIMUI_TEXTWIDGET_STATICTEXT                 = 0,
    SIMUI_TEXTWIDGET_RPMS                       = 1,
    SIMUI_TEXTWIDGET_GEAR                       = 2,
    SIMUI_TEXTWIDGET_LAP                        = 3,
    SIMUI_TEXTWIDGET_LAPS                       = 4,
    SIMUI_TEXTWIDGET_POSITION                   = 5,
    SIMUI_TEXTWIDGET_NUMCARS                    = 6,
    SIMUI_TEXTWIDGET_BESTLAP                    = 7,
    SIMUI_TEXTWIDGET_LASTLAP                    = 8,
    SIMUI_TEXTWIDGET_BRAKEBIAS                  = 9,
    SIMUI_TEXTWIDGET_FUELREMAINING              = 10,
    SIMUI_TEXTWIDGET_VELOCITY                   = 11,
    SIMUI_TEXTWIDGET_POSITION_STANDALONE        = 12,
    SIMUI_TEXTWIDGET_LAP_STANDALONE             = 13,
}
SimUIWidgetSubType;

typedef struct
{
    SimUIWidgetType uiwidgettype;
    SimUIWidgetSubType uiwidgetsubtype;
    int fontid;
    int xpos;
    int ypos;
    int red;
    int green;
    int blue;
    char* text;
    //char* name;
}
SimUIWidget;

int freesettings(SMSettings* sms);
int getuiconfigtouse(const char* config_file_str, char* car, int sim);
int strcicmp(char const* a, char const* b);
int uiconfigcheck(const char* config_file_str, int confignum, int* fonts, int* widgets);
int uiloadconfig(const char* config_file_str, int confignum, FontInfo* fi, SimUIWidget* simuiwidgets, const char* fontpath);
int loaddbconfig(const char* config_file_str, SMSettings* p);
#endif
