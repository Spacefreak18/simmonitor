#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <stdbool.h>

#include "../simulatorapi/simapi/simapi/simapi.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"

typedef struct
{
    char* ui_string;
    int   program_action;
    int   program_state;
    int   port;
    char* config_dirpath;
    char* data_dirpath;
    char* cache_dirpath;
    char* gnuplot_file;
    char* gnuplot_bin;

    char* ui_config_filepath;
    char* db_config_filepath;
    char* css_filepath;

    char* log_filename_str;
    char* log_fullfilename_str;
    char* log_dirname_str;
    bool user_specified_uiconfig_file;
    bool user_specified_dbconfig_file;
    bool user_specified_log_file;
    bool user_specified_css_file;
    bool user_specified_config_dir;
    bool user_specified_data_dir;
    bool user_specified_cache_dir;


    bool  monitor;
    bool  udp;
    int   verbosity_count;
    int err;

    char* db_user;
    char* db_serv;
    char* db_dbnm;
    char* db_pass;
    char* db_conn;

}
Parameters;

typedef enum
{
    A_PLAY          = 0,
    A_BROWSE        = 1,
}
ProgramAction;

typedef enum
{
    E_SUCCESS_AND_EXIT = 0,
    E_SUCCESS_AND_DO   = 1,
    E_SOMETHING_BAD    = 2
}
ConfigError;

typedef enum
{
    E_NO_ERROR         = 0,
    E_BAD_CONFIG       = 1,
    E_FAILED_DB_CONN   = 2,
    E_DB_QUERY_FAIL    = 3,
    E_DB_UNSUPPORTED   = 4,
}
SimMonitorError;


ConfigError getParameters(int argc, char** argv, Parameters* p);
int freeparams(Parameters* p);

struct _errordesc
{
    int  code;
    char* message;
} static errordesc[] =
{
    { E_SUCCESS_AND_EXIT, "No error and exiting" },
    { E_SUCCESS_AND_DO,   "No error and continuing" },
    { E_SOMETHING_BAD,    "Something bad happened" },
};

#endif
