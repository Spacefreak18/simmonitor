#include <basedir.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <libconfig.h>
#include <pthread.h>
#include <basedir_fs.h>


#include "gameloop/gameloop.h"
#include "gameloop/browseloop.h"
#include "helper/parameters.h"
#include "helper/dirhelper.h"
#include "helper/confighelper.h"
#include "slog/src/slog.h"

#define PROGRAM_NAME "simmonitor"

void SetSettingsFromParameters(Parameters* p, SMSettings* sms, char* configdir_str, char* cachedir_str, char* datadir_str)
{

    if(p->user_specified_data_dir == true)
    {
        sms->datadir_str = strdup(p->data_dirpath);
    }
    else
    {
        sms->datadir_str = strdup(datadir_str);
    }

    if(p->user_specified_dbconfig_file == true && does_file_exist(p->db_config_filepath))
    {
        sms->dbconfig_str = strdup(p->db_config_filepath);
    }
    else
    {
        if(p->user_specified_config_dir == true && does_directory_exist(p->config_dirpath))
        {
            asprintf(&sms->dbconfig_str, "%s/%s", p->config_dirpath, "simmonitordb.config");
        }
        else
        {
            asprintf(&sms->dbconfig_str, "%s%s", configdir_str, "simmonitordb.config");
        }
    }

    if(p->user_specified_uiconfig_file == true && does_file_exist(p->ui_config_filepath))
    {
        sms->uiconfig_str = strdup(p->ui_config_filepath);
    }
    else
    {
        if(p->user_specified_config_dir == true && does_directory_exist(p->config_dirpath))
        {
            asprintf(&sms->uiconfig_str, "%s/%s", p->config_dirpath, "simmonitorui.config");
        }
        else
        {
            asprintf(&sms->uiconfig_str, "%s%s", configdir_str, "simmonitorui.config");
        }
    }

    if(p->user_specified_css_file == true && does_file_exist(p->css_filepath))
    {
        sms->css_file_str = strdup(p->css_filepath);
    }
    else
    {
        if(p->user_specified_data_dir == true  && does_directory_exist(p->data_dirpath))
        {
            asprintf(&sms->css_file_str, "%s/%s", p->data_dirpath, "simstyle.css");
        }
        else
        {
            asprintf(&sms->css_file_str, "%s%s", datadir_str, "simstyle.css");
        }
    }

    if(p->user_specified_log_file == true && does_file_exist(p->log_fullfilename_str))
    {
        sms->log_dirname_str = strdup(p->log_dirname_str);
        sms->log_filename_str = strdup(p->log_filename_str);
    }
    else
    {
        sms->log_dirname_str = strdup(cachedir_str);
        sms->log_filename_str = strdup("simmonitor.log");
    }

    sms->mysql = p->mysql;
    sms->force_udp_mode = p->udp;
    if (p->program_action == A_PLAY)
    {
        sms->ui_type = SIMMONITOR_CLI;
        if(p->ui_string != NULL)
        {
            if (strcicmp(p->ui_string, "x") == 0)
            {
                sms->ui_type = SIMMONITOR_X;
            }
            if (strcicmp(p->ui_string, "web") == 0)
            {
                sms->ui_type = SIMMONITOR_WEB;
            }
            if (strcicmp(p->ui_string, "fb") == 0)
            {
                sms->ui_type = SIMMONITOR_FB;
            }
            if (strcicmp(p->ui_string, "curses") == 0)
            {
                sms->ui_type = SIMMONITOR_CURSES;
            }
        }
    }

}

int main(int argc, char** argv)
{

    char* home_dir_str = gethome();
    if(home_dir_str == NULL)
    {
        fprintf(stderr, "You need a home directory");
        return 0;
    }
    Parameters* p = malloc(sizeof(Parameters));
    SMSettings* sms = malloc(sizeof(SMSettings));;
    ConfigError ppe = getParameters(argc, argv, p);
    if (ppe == E_SUCCESS_AND_EXIT)
    {
        goto cleanup_final;
    }
    sms->program_action = p->program_action;
    p->program_state = 1;

    xdgHandle xdg;
    if(!xdgInitHandle(&xdg))
    {
        fprintf(stderr, "Function xdgInitHandle() failed, is $HOME unset?");
    }

    const char* config_home_str = xdgConfigHome(&xdg);
    const char* cache_home_str = xdgCacheHome(&xdg);
    const char* data_home_str = xdgDataHome(&xdg);

    char* cachedir_str = NULL;
    char* configdir_str = NULL;
    char* datadir_str = NULL;

    if((p->user_specified_uiconfig_file == false || p->user_specified_dbconfig_file == false) &&
            p->user_specified_config_dir == false)
    {
        create_xdg_dir(config_home_str);
        configdir_str = create_user_dir(home_dir_str, ".config", PROGRAM_NAME);
    }
    if((p->user_specified_data_dir == false || p->user_specified_css_file == false) &&
            p->user_specified_data_dir == false)
    {
        create_xdg_dir(data_home_str);
        datadir_str = create_user_dir(home_dir_str, ".local/share", PROGRAM_NAME);
    }
    if(p->user_specified_log_file == false)
    {
        create_xdg_dir(cache_home_str);
        cachedir_str = create_user_dir(home_dir_str, ".cache", PROGRAM_NAME);
    }

    SetSettingsFromParameters(p, sms, configdir_str, cachedir_str, datadir_str);
    freeparams(p);
    free(p);

    slog_config_t slgCfg;
    slog_config_get(&slgCfg);
    slgCfg.eColorFormat = SLOG_COLORING_TAG;
    slgCfg.eDateControl = SLOG_TIME_ONLY;
    strcpy(slgCfg.sFileName, sms->log_filename_str);
    strcpy(slgCfg.sFilePath, sms->log_dirname_str);
    slgCfg.nTraceTid = 0;
    slgCfg.nToScreen = 1;
    slgCfg.nUseHeap = 0;
    slgCfg.nToFile = 1;
    slgCfg.nFlush = 0;
    slgCfg.nFlags = SLOG_FLAGS_ALL;
    slog_config_set(&slgCfg);
    if (p->verbosity_count < 2)
    {
        slog_disable(SLOG_TRACE);
    }
    if (p->verbosity_count < 1)
    {
        slog_disable(SLOG_DEBUG);
    }
    slogi("Logging enabled");

    xdgWipeHandle(&xdg);

    if (sms->program_action == A_PLAY)
    {
        slogd("simmonitor will display with ui type %i and config file %s", sms->ui_type, sms->uiconfig_str);

        if(sms->mysql == true)
        {
            slogi("attempting load of db config file %s", sms->dbconfig_str);
            loaddbconfig(sms->dbconfig_str, sms);
        }
        //slogd("css file %s", sms->css_file_str);

        mainloop(sms);
    }
    else
    {
        loaddbconfig(sms->dbconfig_str, sms);
        browseloop(sms, sms->datadir_str);
    }


cleanup_final:
    freesettings(sms);
    free(sms);

    exit(0);
}

