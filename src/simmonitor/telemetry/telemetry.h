#ifndef _TELEMETRY_H
#define _TELEMETRY_H

int dumptelemetrytofile(struct _h_connection* conn, char* datadir, int lap1id, int lap2id);

void print_bytes(void* ptr, int size);

int updatetelemetrydata(struct _h_connection* conn, int tracksamples, int telemid, int lapid,
                        int* speeddata, int* rpmdata, int* geardata,
                        double* steerdata, double* acceldata, double* brakedata);

#endif
