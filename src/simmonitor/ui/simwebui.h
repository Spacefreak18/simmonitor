#include <uv.h>

#include <microhttpd.h>

#include "../gameloop/loopdata.h"

int webuistart(loop_data* l);

struct handler_param
{
    const char* response_page;
};

enum MHD_Result ahc_echo (void* cls, struct MHD_Connection* connection, const char* url,
                          const char* method, const char* version, const char* upload_data, size_t* upload_data_size,
                          void** req_cls);

char *fslurp(FILE *fp);

int webuistop(struct MHD_Daemon* d);
