#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include "../db/hoeldb.h"

int dumptelemetrytofile(struct _h_connection* conn, char* datadir, int lap1id, int lap2id);
int dumplapstofile(char* datadir, SessionDbo sess, LapDbo lapsdb, int numlaps, int sessidx);

void print_bytes(void* ptr, int size);

int updatetelemetrydata(struct _h_connection* conn, int tracksamples, int telemid, int lapid,
                        int* speeddata, int* rpmdata, int* geardata,
                        double* steerdata, double* acceldata, double* brakedata);

#endif
