#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hoel.h>
#include <jansson.h>
#include <uv.h>

#include "telemetry.h"
#include "../db/hoeldb.h"
#include "../gameloop/loopdata.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"

#define TRACK_SAMPLE_RATE        4

struct _h_result result;
struct _h_connection* conn;

int pitstatus = 0;
int sessionstatus = 0;
int lastpitstatus = 0;
int lastsessionstatus = 0;
int lap = 0;
int lastlap = 0;
int midlap = 0;
int sector = 0;
int lastsector = 0;
int lastpos = 0;
int chequeredflag = 0;

int eventid;
int driverid;
int carid;

int stintid = 0;
int stintlapid = 0;
int sessionid = 0;

int go = true;
char lastsimstatus = false;
int tick = 0;

int track_samples = 0;
int stintlaps = 1;
int validstintlaps = 0;
bool validind = true;
double maxspeed = 0;
double avgspeed = 0;

int* speeddata;
int* rpmdata;
int* geardata;
double* steerdata;
double* acceldata;
double* brakedata;

int sectortimes[4];

void telemetryinit(SimData* SimData, SimMap* simmap, SMSettings* sms)
{
    slogt("Attempting database connection");
    switch ( sms->db_type )
    {
        case HOEL_DB_TYPE_PGSQL:
            conn = h_connect_pgsql(sms->db_conn);
            break;
        case HOEL_DB_TYPE_SQLITE:
            conn = h_connect_sqlite(sms->db_conn);
            break;
        default:
            sms->dberr = E_DB_UNSUPPORTED;
            return;
    }

    if (conn == NULL)
    {
        slogf("Unable to connect to configured SimMonitor database. Are the parameters in the config correct? Is the user allowed to access from this address?");
        sms->dberr = E_FAILED_DB_CONN;
    }
    slogi("Starting telemetry");

    slogt("Getting track config for track %s", SimData->track);
    int trackconfig = gettrack(conn, SimData->track);
    if (trackconfig == -1)
    {
        slogf("Problem performing select query. Does the db user have read permissions?");
        sms->dberr = E_FAILED_DB_CONN;
    }

    slogt("Adding track if necessary");
    trackconfig = addtrackconfig(conn, trackconfig, SimData->track, SimData->trackdistancearound);
    if (trackconfig == -1)
    {
        slogf("Problem performing insert query. Does the db user have write permissions?");
        sms->dberr = E_FAILED_DB_CONN;
    }
    slogt("Detected track configuration id: %i", trackconfig);
    eventid = addevent(conn, trackconfig);
    driverid = getdriver(conn, SimData->driver);
    driverid = adddriver(conn, driverid, SimData->driver);
    carid = getcar(conn, SimData->car);
    carid = addcar(conn, carid, SimData->car);

    // ?? close last session

    pitstatus = SimData->cars[0].inpit;
    sessionstatus = 0;
    lastpitstatus = pitstatus;
    lastsessionstatus = 0;
    lap = 0;
    lastlap = 0;
    sector = 0;
    lastsector = 0;
    lastpos = 0;

    stintid = 0;
    stintlapid = 0;
    sessionid = 0;

    slogt("Starting session with sim exe %i", SimData->simexe);
    sessionid = addsession(conn, eventid, SimData->simexe, driverid, carid, SimData->session, SimData->airtemp, SimData->tracktemp, SimData);
    stintid = addstint(conn, sessionid, driverid, carid, SimData);
    stintlapid = addstintlap(conn, stintid, SimData);
    sessionstatus = SimData->session;
    lastsessionstatus = sessionstatus;
    lap = SimData->lap;
    lastlap = lap;
    go = true;
    lastsimstatus = false;
    tick = 0;


    //track_samples = SimData->trackspline * TRACK_SAMPLE_RATE;
    track_samples = SimData->tracksamples;
    slogt("track samples %i", track_samples);

    stintlaps = 1;
    validstintlaps = 0;
    validind = true;

    maxspeed = 0;
    avgspeed = 0;

    speeddata = calloc(track_samples, sizeof(SimData->velocity));
    rpmdata = calloc(track_samples, sizeof(SimData->rpms));
    geardata = calloc(track_samples, sizeof(SimData->gear));
    steerdata = calloc(track_samples, sizeof(SimData->steer));
    acceldata = calloc(track_samples, sizeof(SimData->gas));
    brakedata = calloc(track_samples, sizeof(SimData->brake));

    int sectortimes[4];
}

void telemetrycallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* SimData = f->simdata;
    SimMap* simmap = f->simmap;
    int pos = (int) track_samples * SimData->playerspline;
    slogt("tick %i pos %f normpos %i of samples %i", tick, SimData->playerspline, pos, track_samples);

    steerdata[pos] = SimData->steer;
    acceldata[pos] = SimData->gas;
    brakedata[pos] = SimData->brake;
    speeddata[pos] = SimData->velocity;
    rpmdata[pos] = SimData->rpms;
    geardata[pos] = SimData->gear;
    if(SimData->playerflag == SIMAPI_FLAG_CHEQUERED || SimData->courseflag == SIMAPI_FLAG_CHEQUERED)
    {
        chequeredflag = 1;
    }

    if(lastpos < pos-1)
    {
        while(lastpos < pos)
        {
            steerdata[lastpos] = SimData->steer;
            acceldata[lastpos] = SimData->gas;
            brakedata[lastpos] = SimData->brake;
            speeddata[lastpos] = SimData->velocity;
            rpmdata[lastpos] = SimData->rpms;
            geardata[lastpos] = SimData->gear;
            lastpos++;
        }
    }
    lastpos = pos;

    if ( speeddata[pos] > maxspeed )
    {
        maxspeed = speeddata[pos];
    }

    // in case you are in the middle of an execution and the sim has stopped
    if ((SimData->simstatus == 2 && chequeredflag == 0) || midlap == 1)
    {
        avgspeed += (double) speeddata[pos];;

        slogt("speed %i rpms %i gear %i steer %f gas %f brake %f avgspeed %f", speeddata[pos], rpmdata[pos], geardata[pos], steerdata[pos],
              acceldata[pos], brakedata[pos], avgspeed);

        sessionstatus = SimData->session;
        lap = SimData->lap;
        sector = SimData->sectorindex;


        if (SimData->lapisvalid == false && validind == true)
        {
            validind = false;
        }
        sectortimes[SimData->sectorindex] = SimData->lastsectorinms;
        if (sessionstatus != lastsessionstatus)
        {
            closestint(conn, stintid, stintlaps, validstintlaps);
            closesession(conn, sessionid);
            if(SimData->simstatus == 2 && chequeredflag == 0 && SimData->session > 1)
            {
                slogd("Adding new session");
                sessionid = addsession(conn, eventid, SimData->simexe, driverid, carid, SimData->session, SimData->airtemp, SimData->tracktemp, SimData);
            }

            //pitstatus = 1;
            stintlaps = 1;
            validstintlaps = 0;
        }

        pitstatus = SimData->cars[0].inpit;
        if (pitstatus == true && pitstatus != lastpitstatus)
        {
            //pitstatus = 1;
            //}
            //if (pitstatus = 0 && pitstatus != lastpitstatus)
            //{
            // close last stint

            closestint(conn, stintid, stintlaps, validstintlaps);
            if(SimData->simstatus == 2 && chequeredflag == 0)
            {
                slogd("Adding new stint");
                stintid = addstint(conn, sessionid, driverid, carid, SimData);
            }
            stintlaps = 1;
            validstintlaps = 0;
        }

        if (lap != lastlap)
        {
            slogd("New lap detected");

            midlap = 0;
            if(SimData->simstatus == 2 && chequeredflag == 0)
            {
                midlap = 1;
            }
            if (validind == true)
            {
                validstintlaps++;
            }

            avgspeed = avgspeed / tick;
            closelap(conn, stintlapid, stintid, sectortimes[1], sectortimes[2], SimData->lastsectorinms, 0, 0, maxspeed, avgspeed, SimData);
            // pit status check should be here
            // if car is in pit close previous stint and start a new one

            int telemid = addtelemetry(conn, track_samples, stintlapid);
            int b = updatetelemetrydata(conn, track_samples, telemid, stintlapid, speeddata, rpmdata, geardata, steerdata, acceldata, brakedata);
            if(SimData->simstatus == 2 && chequeredflag == 0)
            {
                slogd("Starting new lap");
                stintlaps++;
                stintlapid = addstintlap(conn, stintid, SimData);
            }




            // reset tracking variables
            validind = true; // assume lap is valid until it isn't
            maxspeed = 0;
            avgspeed = 0;
            tick = 0;


            speeddata = calloc(track_samples, sizeof(SimData->velocity));
            rpmdata = calloc(track_samples, sizeof(SimData->rpms));
            geardata = calloc(track_samples, sizeof(SimData->gear));
            steerdata = calloc(track_samples, sizeof(SimData->steer));
            acceldata = calloc(track_samples, sizeof(SimData->gas));
            brakedata = calloc(track_samples, sizeof(SimData->brake));
        }

        lastpitstatus = pitstatus;
        lastsessionstatus = sessionstatus;
        lastsector = sector;
        lastlap = lap;
        tick++;
    }




    if (f->uion == false)
    {
        uv_timer_stop(handle);
    }
}

void telemetrystop(SimData* SimData)
{

    slogd("telemetry stop signal current lapid %i current stintid %i current sessionid %i", stintlapid, stintid, sessionid);
    //int telemid = addtelemetry(conn, track_samples, stintlapid);
    //int b = updatetelemetrydata(conn, track_samples, telemid, stintlapid, speeddata, rpmdata, geardata, steerdata, acceldata, brakedata);
    //avgspeed = avgspeed / tick;
    //closelap(conn, stintlapid, stintid, sectortimes[1], sectortimes[2], SimData->lastsectorinms, 0, 0, maxspeed, avgspeed, SimData);
    closestint(conn, stintid, stintlaps, validstintlaps);
    closesession(conn, sessionid);

    h_close_db(conn);
    h_clean_connection(conn);

    free(brakedata);
    free(geardata);
    free(rpmdata);
    free(steerdata);
    free(speeddata);
    free(acceldata);

    slogd("telemetry data closed");
}
