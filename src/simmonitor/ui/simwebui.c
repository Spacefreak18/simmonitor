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
  "<script src=\"simscript.js\"></script>" \
  "</head><body><div id=\"maindash\" hx-get=\"/simdata\" hx-trigger=\"every 7ms\" hx-swap=\"innerHTML\" hx-on::after-swap=\"simScript()\" ><p>searching for sim data</p>" \
  "</div></body></html>"

SimData* sd;
SimMap* sm;
SimUIWidget* simuiwidgets;
char* css;
char* js;
char* templatefile;
int numwidgets;

char *fslurp(FILE *fp)
{
  char *answer;
  char *temp;
  int buffsize = 1024;
  int i = 0;
  int ch;

  answer = malloc(1024);
  if(!answer)
    return 0;
  while( (ch = fgetc(fp)) != EOF )
  {
    if(i == buffsize-2)
    {
      if(buffsize > INT_MAX - 100 - buffsize/10)
      {
    free(answer);
        return 0;
      }
      buffsize = buffsize + 100 * buffsize/10;
      temp = realloc(answer, buffsize);
      if(temp == 0)
      {
        free(answer);
        return 0;
      }
      answer = temp;
    }
    answer[i++] = (char) ch;
  }
  answer[i++] = 0;

  temp = realloc(answer, i);
  if(temp)
    return temp;
  else
    return answer;
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
        slogd("css file requested");
    }
    else
    {
        if ( strcmp( "/simscript.js", url ) == 0 )
        {
            response = MHD_create_response_from_buffer_static (strlen (js), js);
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);
            slogd("javascript file requested");
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
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_FLAG)
                    {
                        char* txt1;
                        asprintf(&txt1, "%i", sd->playerflag);
                        vl = TMPL_add_var(0, "datum", "flag", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
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
                        txt1 = sd->gearc;
                        vl = TMPL_add_var(0, "datum", "gear", "data", txt1, 0);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_VELOCITY)
                    {
                        char* txt1;
                        asprintf(&txt1, "%i", sd->velocity);
                        vl = TMPL_add_var(0, "datum", "velocity", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_LAP)
                    {
                        char* txt1;
                        asprintf(&txt1, "%i / %i", sd->lap, sd->numlaps);
                        vl = TMPL_add_var(0, "datum", "lap", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_POSITION)
                    {
                        char* txt1;
                        asprintf(&txt1, "%i / %i", sd->position, sd->numcars);
                        vl = TMPL_add_var(0, "datum", "position", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_FUELREMAINING)
                    {
                        char* txt1;
                        asprintf(&txt1, "%f", sd->fuel);
                        vl = TMPL_add_var(0, "datum", "fuel", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_LASTLAP)
                    {
                        char* txt1;
                        asprintf(&txt1, "%d:%02d:%03d\n", sd->lastlap.minutes, sd->lastlap.seconds, sd->lastlap.fraction);
                        vl = TMPL_add_var(0, "datum", "lastlap", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_BESTLAP)
                    {
                        char* txt1;
                        asprintf(&txt1, "%d:%02d:%03d\n", sd->bestlap.minutes, sd->bestlap.seconds, sd->bestlap.fraction);
                        vl = TMPL_add_var(0, "datum", "bestlap", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                    if (simuiwidgets[j].uiwidgetsubtype == SIMUI_TEXTWIDGET_CURRENTLAP)
                    {
                        char* txt1;
                        asprintf(&txt1, "%d:%02d:%03d\n", sd->currentlap.minutes, sd->currentlap.seconds, sd->currentlap.fraction);
                        vl = TMPL_add_var(0, "datum", "currentlap", "data", txt1, 0);
                        free(txt1);
                        loop = TMPL_add_varlist(loop, vl);
                    }
                }
                mylist = TMPL_add_loop(0, "myloop", loop);

                FILE* stream;
                size_t size;

                stream = open_memstream (&html, &size);

                TMPL_write(templatefile, 0, 0, mylist, stream, stderr);

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
    }

    return ret;
}

int webuistart(loop_data* l)
{
    sd = l->simdata;
    sm = l->simmap;
    css = l->css;
    js = l->js;
    simuiwidgets = l->simuiwidgets;
    numwidgets = l->numwidgets;
    templatefile = l->templatefile;
    slogd("Using template file %s", templatefile);
    return 0;
}

int webuistop(struct MHD_Daemon* d)
{
    sd = NULL;
    sm = NULL;
    css = NULL;
    js = NULL;
    MHD_stop_daemon(d);
    return 0;
}
