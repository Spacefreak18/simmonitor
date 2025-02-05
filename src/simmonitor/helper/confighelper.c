#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <hoel.h>

#include "confighelper.h"

#include "../slog/src/slog.h"
#include "parameters.h"

int freesettings(SMSettings* sms)
{
    if(sms->uiconfig_str != NULL)
    {
        free(sms->uiconfig_str);
    }
    if(sms->dbconfig_str != NULL)
    {
        free(sms->dbconfig_str);
    }
    if(sms->css_file_str != NULL)
    {
        free(sms->css_file_str);
    }
    if(sms->datadir_str != NULL)
    {
        free(sms->datadir_str);
    }
    if(sms->cachedir_str != NULL)
    {
        free(sms->cachedir_str);
    }
    if(sms->log_filename_str != NULL)
    {
        free(sms->log_filename_str);
    }
    if(sms->log_dirname_str != NULL)
    {
        free(sms->log_dirname_str);
    }
    if(sms->gnuplot_file_str != NULL)
    {
        free(sms->gnuplot_file_str);
    }
    if(sms->gnuplot_bin_str != NULL)
    {
        free(sms->gnuplot_bin_str);
    }
    if(sms->db_user != NULL)
    {
        free(sms->db_user);
    }
    if(sms->db_pass != NULL)
    {
        free(sms->db_pass);
    }
    if(sms->db_serv != NULL)
    {
        free(sms->db_serv);
    }
    if(sms->db_dbnm != NULL)
    {
        free(sms->db_dbnm);
    }
    if(sms->db_conn != NULL)
    {
        free(sms->db_conn);
    }

    return 0;
}

int strcicmp(char const* a, char const* b)
{
    for (;; a++, b++)
    {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
        {
            return d;
        }
    }
}

int getsubtype(char const* a)
{
    int r = SIMUI_TEXTWIDGET_STATICTEXT;

    r = strcicmp("gear", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_GEAR;
    }

    r = strcicmp("rpm", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_RPMS;
    }

    r = strcicmp("flag", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_FLAG;
    }

    r = strcicmp("velocity", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_VELOCITY;
    }

    r = strcicmp("pos_standalone", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_POSITION_STANDALONE;
    }

    r = strcicmp("pos", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_POSITION;
    }

    r = strcicmp("numcars", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_NUMCARS;
    }

    r = strcicmp("lap_standalone", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_LAP_STANDALONE;
    }

    r = strcicmp("lap", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_LAP;
    }

    r = strcicmp("lastlap", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_LASTLAP;
    }

    r = strcicmp("bestlap", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_BESTLAP;
    }

    r = strcicmp("currentlap", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_CURRENTLAP;
    }

    r = strcicmp("brakebias", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_BRAKEBIAS;
    }

    r = strcicmp("fuelremaining", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_FUELREMAINING;
    }

    r = strcicmp("laps", a);
    if (r == 0)
    {
        return SIMUI_TEXTWIDGET_LAPS;
    }

    return SIMUI_TEXTWIDGET_STATICTEXT;
}

int getuiconfigtouse(const char* config_file_str, char* car, int sim)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        return -1;
    }
    config_setting_t* config = NULL;
    config_setting_t* config_widgets = NULL;
    config = config_lookup(&cfg, "configs");
    int configs = config_setting_length(config);

    const char* temp;
    config_setting_t* config_config = NULL;
    int j = 0;
    if ( configs == 1 )
    {
        return 0;
    }
    int confignum = 0;
    for (j = 0; j < configs; j++)
    {
        config_config = config_setting_get_elem(config, j);

        int found = 0;
        int csim = 0;
        config_setting_lookup_int(config_config, "sim", &csim);
        if (csim != sim && csim != 0)
        {
            continue;
        }
        found = config_setting_lookup_string(config_config, "car", &temp);
        if(strcicmp("default", car) == 0)
        {
            confignum = j;
        }
        if(strcicmp(temp, car) != 0)
        {
            continue;
        }
        confignum = j;

        break;
    }
    return confignum;
}

int loadconnectionconfig(config_t* cfg, SMSettings* sms)
{
    config_setting_t* config_db_array = NULL;
    config_db_array = config_lookup(cfg, "db");

    config_setting_t* config_db = NULL;
    config_db = config_setting_get_elem(config_db_array, 0);

    if (config_db == NULL)
    {
        slogi("found no db settings");
        return E_BAD_CONFIG;
    }

    int dbtype = 0;
    config_setting_lookup_int(config_db, "type", &dbtype);

    const char* temp;
    if (dbtype > HOEL_DB_TYPE_SQLITE)
    {
        sms->db_type = HOEL_DB_TYPE_PGSQL;
        slogt("config file defines a postgres database");
        config_setting_lookup_string(config_db, "user", &temp);
        sms->db_user = strdup(temp);
        config_setting_lookup_string(config_db, "password", &temp);
        sms->db_pass = strdup(temp);
        config_setting_lookup_string(config_db, "server", &temp);
        sms->db_serv = strdup(temp);
        config_setting_lookup_string(config_db, "database", &temp);
        sms->db_dbnm = strdup(temp);

        asprintf(&sms->db_conn, "host=%s dbname=%s user=%s password=%s", sms->db_serv, sms->db_dbnm, sms->db_user, sms->db_pass);
    }
    else
    {
        sms->db_type = HOEL_DB_TYPE_SQLITE;
        slogt("config file defines a sqlite database");
        config_setting_lookup_string(config_db, "database", &temp);
        sms->db_serv = strdup(temp);
        sms->db_conn = strdup(temp);
    }
    return E_NO_ERROR;
}

int loaddbconfig(const char* config_file_str, SMSettings* p)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    }
    else
    {
        slogi("Parsing config file");


        if (p->program_action == A_BROWSE)
        {
            char* gnuplotbin = "/usr/bin/gnuplot";
            config_setting_t* config_graph_array = NULL;
            config_graph_array = config_lookup(&cfg, "graph");

            config_setting_t* config_graph = NULL;
            config_graph = config_setting_get_elem(config_graph_array, 0);

            if (config_graph == NULL)
            {
                slogi("found no graph settings");
                return E_BAD_CONFIG;
            }
            const char* temp;
            config_setting_lookup_string(config_graph, "gnuplotfile", &temp);
            p->gnuplot_file_str = strdup(temp);
            slogt("set gnuplot config file %s", p->gnuplot_file_str);

            int found = config_setting_lookup_string(config_graph, "gnuplotbin", &temp);
            if (found > 0)
            {
                p->gnuplot_bin_str = strdup(temp);
            }
            else
            {
                p->gnuplot_bin_str = strdup(gnuplotbin);
            }
            p->gnuplotfound = 0;
            if (access(p->gnuplot_bin_str, F_OK) == 0)
            {
                p->gnuplotfound = 1;
            }
        }

        if (p->mysql == true || p->program_action == A_BROWSE)
        {
            int err = loadconnectionconfig(&cfg, p);
            if (err != E_NO_ERROR)
            {
                return err;
            }
        }
    }

    config_destroy(&cfg);

    return 0;
}

int uiconfigcheck(const char* config_file_str, int confignum, int* fonts, int* widgets)
{
    slogt("ui config check");
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    }

    config_setting_t* config = NULL;
    config = config_lookup(&cfg, "configs");
    config_setting_t* selectedconfig = config_setting_get_elem(config, confignum);
    slogt("selected num %i", confignum);
    config_setting_t* config_fonts = NULL;
    config_setting_t* config_widgets = NULL;
    config_fonts = config_setting_lookup(selectedconfig, "fonts_array");
    *fonts = config_setting_length(config_fonts);
    config_widgets = config_setting_lookup(selectedconfig, "widgets_array");
    *widgets = config_setting_length(config_widgets);
    config_destroy(&cfg);
    return 0;
    //return cfg;
}

int uiloadconfig(const char* config_file_str, int confignum, FontInfo* fi, SimUIWidget* simuiwidgets, const char* fontpath, SMSettings* sms)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    }
    else
    {
        slogi("Parsing config file");

        config_setting_t* config = NULL;
        config = config_lookup(&cfg, "configs");
        config_setting_t* selectedconfig = config_setting_get_elem(config, confignum);
        config_setting_t* config_font = NULL;
        config_setting_t* config_fonts = NULL;
        config_fonts = config_setting_lookup(selectedconfig, "fonts_array");
        int fontslen = config_setting_length(config_fonts);
        slogd("will attempt to read %i fonts in fontpath %s", fontslen, fontpath);
        //sms->fonts_length = fontslen;


        const char* temp;
        for (int j = 0; j < fontslen; j++)
        {
            config_font = config_setting_get_elem(config_fonts, j);

            int found = config_setting_lookup_string(config_font, "name", &temp);
            found = config_setting_lookup_int(config_font, "size", &fi[j].size);
            // TODO: check if these are already full paths or just don't allow full paths and check multiple paths
            // better yet, find a library
            slogi("found font %s", temp);
            size_t strzie = strlen(fontpath) + strlen(temp) + 1 + 1;
            char* temp2 = malloc(strzie);
            snprintf(temp2, strzie, "%s/%s", fontpath, temp);
            fi[j].name = strdup(temp2);
            free(temp2);
        }

        int found = config_setting_lookup_string(selectedconfig, "webdeffile", &temp);
        if( found > 0 )
        {
            sms->web_def_file = strdup(temp);
            slogt("found web definition file %s", sms->web_def_file);
        }

        config_setting_t* config_widget = NULL;
        config_setting_t* config_widgets = NULL;
        config_widgets = config_setting_lookup(selectedconfig, "widgets_array");
        int widgetslen = config_setting_length(config_widgets);
        slogd("will attempt to read %i widgets", widgetslen);
        //sms->widgets_length = widgetslen;


        for (int j = 0; j < widgetslen; j++)
        {
            config_widget = config_setting_get_elem(config_widgets, j);

            int found = config_setting_lookup_string(config_widget, "name", &temp);
            found = config_setting_lookup_string(config_widget, "type", &temp);
            simuiwidgets[j].uiwidgettype = SIMUI_WIDGET_TEXT;

            found = config_setting_lookup_string(config_widget, "subtype", &temp);
            simuiwidgets[j].uiwidgetsubtype = getsubtype(temp);
            if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_STATICTEXT)
            {
                found = config_setting_lookup_string(config_widget, "text", &temp);
                simuiwidgets[j].text = strdup(temp);
            }

            found = config_setting_lookup_int(config_widget, "fontid", &simuiwidgets[j].fontid);
            found = config_setting_lookup_int(config_widget, "xpos", &simuiwidgets[j].xpos);
            found = config_setting_lookup_int(config_widget, "ypos", &simuiwidgets[j].ypos);
            found = config_setting_lookup_int(config_widget, "r", &simuiwidgets[j].red);
            found = config_setting_lookup_int(config_widget, "g", &simuiwidgets[j].green);
            found = config_setting_lookup_int(config_widget, "b", &simuiwidgets[j].blue);
        }

    }
    config_destroy(&cfg);

    return 0;
}
