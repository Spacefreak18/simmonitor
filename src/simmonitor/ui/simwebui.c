#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <microhttpd.h>

#include "../gameloop/loopdata.h"
#include "../ctemplate/ctemplate.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"

#define PAGE \
  "<html><head><title>simdash</title>" \
  "<script src=\"https://unpkg.com/htmx.org@2.0.1\" integrity=\"sha384-QWGpdj554B4ETpJJC9z+ZHJcA/i59TyjxEPXiiUgN2WmTyV5OEZWCD6gQhgkdpB/\" crossorigin=\"anonymous\"></script>" \
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"simstyle.css\">" \
  "</head><body><div id=\"maindash\" hx-get=\"/simdata\" hx-trigger=\"every 1s\"><p>searching for sim data</p>" \
  "</div></body></html>"

SimData* sd;
SimMap* sm;
SimUIWidget* simuiwidgets;
char* css;
int numwidgets;

long slurp(char const* path, char** buf, bool add_nul)
{
    FILE*  fp;
    size_t fsz;
    long   off_end;
    int    rc;

    fp = fopen(path, "rb");
    if( NULL == fp )
    {
        return -1L;
    }

    rc = fseek(fp, 0L, SEEK_END);
    if( 0 != rc )
    {
        return -1L;
    }

    if( 0 > (off_end = ftell(fp)) )
    {
        return -1L;
    }
    fsz = (size_t)off_end;

    *buf = malloc( fsz+(int)add_nul );
    if( NULL == *buf )
    {
        return -1L;
    }

    rewind(fp);

    if( fsz != fread(*buf, 1, fsz, fp) )
    {
        free(*buf);
        return -1L;
    }

    if( EOF == fclose(fp) )
    {
        free(*buf);
        return -1L;
    }

    if( add_nul )
    {
        buf[fsz] = '\0';
    }

    return (long)fsz;
}


enum MHD_Result ahc_echo (void* cls, struct MHD_Connection* connection, const char* url,
                          const char* method, const char* version, const char* upload_data, size_t* upload_data_size,
                          void** req_cls)
{
    static int aptr;
    struct handler_param* param = (struct handler_param*) cls;
    struct MHD_Response* response;
    enum MHD_Result ret;

    (void) url;               /* Unused. Silent compiler warning. */
    (void) version;           /* Unused. Silent compiler warning. */
    (void) upload_data;       /* Unused. Silent compiler warning. */
    (void) upload_data_size;  /* Unused. Silent compiler warning. */


    if (0 != strcmp (method, "GET"))
    {
        return MHD_NO;
    }

    if (&aptr != *req_cls)
    {
        /* do never respond on first call */
        *req_cls = &aptr;
        return MHD_YES;
    }
    *req_cls = NULL;                  /* reset when done */


    if ( strcmp( "/simstyle.css", url ) == 0 )
    {
        response = MHD_create_response_from_buffer_static (strlen (css), css);
        ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
        MHD_destroy_response (response);

    }
    else
    {

        if ( strcmp( "/simdata", url ) == 0 )
        {

            char* html;
            TMPL_varlist* mylist;

            TMPL_varlist* vl, *mainlist;
            TMPL_loop*    loop;

            /* build the loop variable */

            loop = 0;

            for (int j = 0; j < numwidgets; j++)
            {
                if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_RPMS)
                {
                    char* txt1;
                    asprintf(&txt1, "%i", sd->rpms);
                    vl = TMPL_add_var(0, "datum", "rpm", "data", txt1, 0);
                    free(txt1);
                    loop = TMPL_add_varlist(loop, vl);
                }
                if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_GEAR)
                {
                    char* txt1;
                    asprintf(&txt1, "%c", sd->gearc);
                    vl = TMPL_add_var(0, "datum", "gear", "data", txt1, 0);
                    free(txt1);
                    loop = TMPL_add_varlist(loop, vl);
                }
            }
            mylist = TMPL_add_loop(0, "myloop", loop);

            FILE* stream;
            size_t size;

            stream = open_memstream (&html, &size);

            TMPL_write("base.tmpl", 0, 0, mylist, stream, stderr);

            fflush(stream);

            response = MHD_create_response_from_buffer_copy(strlen(html), html);
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);

            fclose(stream);
            free(html);
            TMPL_free_varlist(mylist);

        }
        else
        {
            response = MHD_create_response_from_buffer_static (strlen (PAGE), PAGE);
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);
        }
    }

    return ret;
}

int webuistart(loop_data* l)
{
    sd = l->simdata;
    sm = l->simmap;
    css = l->css;
    simuiwidgets = l->simuiwidgets;
    numwidgets = l->numwidgets;
    return 0;
}

int webuistop(struct MHD_Daemon* d)
{
    sd = NULL;
    sm = NULL;
    css = NULL;
    MHD_stop_daemon(d);
    return 0;
}
