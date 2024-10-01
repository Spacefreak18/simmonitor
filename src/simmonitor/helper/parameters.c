#include "parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <libconfig.h>

#include <argtable2.h>
#include <regex.h>

int freeparams(Parameters* p)
{

    if(p->config_dirpath != NULL)
    {
        free(p->config_dirpath);
    }
    if(p->data_dirpath != NULL)
    {
        free(p->data_dirpath);
    }
    if(p->gnuplot_file != NULL)
    {
        free(p->gnuplot_file);
    }
    if(p->gnuplot_bin != NULL)
    {
        free(p->gnuplot_bin);
    }
    if(p->ui_config_filepath != NULL)
    {
        free(p->ui_config_filepath);
    }
    if(p->db_config_filepath != NULL)
    {
        free(p->db_config_filepath);
    }
    if(p->css_filepath != NULL)
    {
        free(p->css_filepath);
    }
    if(p->log_filename_str != NULL)
    {
        free(p->log_filename_str);
    }
    if(p->log_fullfilename_str != NULL)
    {
        free(p->log_fullfilename_str);
    }
    if(p->log_dirname_str != NULL)
    {
        free(p->log_dirname_str);
    }

    if(p->db_user != NULL)
    {
        free(p->db_user);
    }
    if(p->db_pass != NULL)
    {
        free(p->db_pass);
    }
    if(p->db_serv != NULL)
    {
        free(p->db_serv);
    }
    if(p->db_dbnm != NULL)
    {
        free(p->db_dbnm);
    }
    if(p->db_conn != NULL)
    {
        free(p->db_conn);
    }
    if(p->gnuplot_file != NULL)
    {
        free(p->gnuplot_file);
    }
    if(p->gnuplot_bin != NULL)
    {
        free(p->gnuplot_bin);
    }
    return 0;
}

ConfigError getParameters(int argc, char** argv, Parameters* p)
{

    ConfigError exitcode = E_SOMETHING_BAD;

    // set return structure defaults
    p->program_action      = 0;
    p->mysql               = false;
    p->udp                 = false;
    p->verbosity_count     = 0;

    p->ui_string           = NULL;
    p->db_user             = NULL;
    p->db_pass             = NULL;
    p->db_serv             = NULL;
    p->db_dbnm             = NULL;
    p->db_conn             = NULL;
    p->gnuplot_file        = NULL;
    p->gnuplot_bin         = NULL;

    p->user_specified_uiconfig_file = false;
    p->user_specified_dbconfig_file = false;
    p->user_specified_log_file = false;
    p->user_specified_css_file = false;
    p->user_specified_config_dir = false;
    p->user_specified_data_dir = false;

    // setup argument handling structures
    const char* progname = "SimMonitor";

    struct arg_lit* arg_verbosity0   = arg_litn("v","verbose", 0, 2, "increase logging verbosity");
    struct arg_lit* arg_verbosity1   = arg_litn("v","verbose", 0, 2, "increase logging verbosity");

    struct arg_rex* cmd1              = arg_rex1(NULL, NULL, "play", NULL, REG_ICASE, NULL);
    struct arg_rex* cmd2              = arg_rex1(NULL, NULL, "browse", NULL, REG_ICASE, NULL);
    struct arg_str* arg_ui            = arg_strn("u", "ui", "<ui>", 0, 1, NULL);
    struct arg_lit* arg_udp           = arg_lit0("d", "udp", "force udp mode for sims which support it");
    struct arg_file* arg_dbconf       = arg_filen(NULL, "dbconf", "<db_config_file>", 0, 1, NULL);
    struct arg_file* arg_uiconf       = arg_filen(NULL, "uiconf", "<ui_config_file>", 0, 1, NULL);
    struct arg_file* arg_css          = arg_filen("c", "css", "<css_file>", 0, 1, NULL);
    struct arg_file* arg_log          = arg_filen("l", "log", "<log_file>", 0, 1, NULL);
    struct arg_str* arg_userdir       = arg_strn(NULL, "userdir", "<user_dir>", 0, 1, NULL);
    struct arg_str* arg_confdir       = arg_strn(NULL, "configdir", "config_dir>", 0, 1, NULL);
    //struct arg_lit* arg_cli           = arg_lit0("c",  "textui", "text only ui");
    //struct arg_lit* arg_mqtt          = arg_lit0("Q",  "mqtt", "send data to local mqtt server with connection settings speciifed in config");
    struct arg_lit* arg_mysql         = arg_lit0("m",  "mysql", "send data to local mysql server with connection settings specified in config");
    struct arg_lit* help0             = arg_litn(NULL,"help", 0, 1, "print this help and exit");
    struct arg_lit* vers0             = arg_litn(NULL,"version", 0, 1, "print version information and exit");
    struct arg_end* end0              = arg_end(20);
    struct arg_lit* help1             = arg_litn(NULL,"help", 0, 1, "print this help and exit");
    struct arg_lit* vers1             = arg_litn(NULL,"version", 0, 1, "print version information and exit");
    struct arg_end* end1              = arg_end(20);
    void* argtable0[]                 = {cmd1,arg_ui,arg_udp,arg_verbosity0,arg_mysql,arg_uiconf,arg_dbconf,arg_css,arg_log,help0,vers0,end0};
    void* argtable1[]                 = {cmd2,arg_verbosity1,help1,vers1,end1};
    int nerrors0;
    int nerrors1;



    if (arg_nullcheck(argtable0) != 0)
    {
        printf("%s: insufficient memory\n",progname);
        goto cleanup;
    }
    if (arg_nullcheck(argtable1) != 0)
    {
        printf("%s: insufficient memory\n",progname);
        goto cleanup;
    }

    nerrors0 = arg_parse(argc,argv,argtable0);
    nerrors1 = arg_parse(argc,argv,argtable1);

    if (nerrors0==0)
    {
        p->program_action = A_PLAY;
        p->ui_string = strdup(arg_ui->sval[0]);
        p->verbosity_count = arg_verbosity0->count;
        //if (arg_mqtt->count > 0)
        //{
        //    p->mqtt = true;
        //}
        if (arg_mysql->count > 0)
        {
            p->mysql = true;
        }
        if (arg_udp->count > 0)
        {
            p->udp = true;
        }

        if(arg_uiconf->count > 0)
        {
            p->ui_config_filepath = strdup(arg_uiconf->filename[0]);
            p->user_specified_uiconfig_file = true;
        }
        if(arg_dbconf->count > 0)
        {
            p->db_config_filepath = strdup(arg_dbconf->filename[0]);
            p->user_specified_dbconfig_file = true;
        }
        if(arg_css->count > 0)
        {
            p->css_filepath = strdup(arg_css->filename[0]);
            p->user_specified_css_file = true;
        }
        if(arg_log->count > 0)
        {
            char* filename = strdup(arg_log->filename[0]);
            p->log_fullfilename_str = strdup(arg_log->filename[0]);
            p->log_filename_str = strdup(arg_log->basename[0]);
            char* dname;
            dname = dirname(filename);
            p->log_dirname_str = strdup(dname);
            p->user_specified_log_file = true;
            free(filename);
        }
        exitcode = E_SUCCESS_AND_DO;
    }
    else
    {
        if (nerrors1==0)
        {
            p->program_action = A_BROWSE;
            //p->sim_string = strdup(arg_sim->sval[0]);
            p->verbosity_count = arg_verbosity1->count;
            exitcode = E_SUCCESS_AND_DO;
        }
    }
    // interpret some special cases before we go through trouble of reading the config file
    if (help0->count > 0)
    {
        printf("Usage: %s\n", progname);
        arg_print_syntax(stdout,argtable0,"\n");
        arg_print_syntax(stdout,argtable1,"\n");
        printf("\nReport bugs on the github github.com/spacefreak18/simmonitor.\n");
        exitcode = E_SUCCESS_AND_EXIT;
        goto cleanup;
    }

    if (vers0->count > 0)
    {
        printf("%s Simulator Monitor\n",progname);
        printf("August 2024, Paul Dino Jones\n");
        exitcode = E_SUCCESS_AND_EXIT;
        goto cleanup;
    }

cleanup:
    arg_freetable(argtable0,sizeof(argtable0)/sizeof(argtable0[0]));
    arg_freetable(argtable1,sizeof(argtable1)/sizeof(argtable1[0]));
    return exitcode;

}
