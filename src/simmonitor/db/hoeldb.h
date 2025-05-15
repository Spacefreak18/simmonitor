
#ifndef _HOELDB_H
#define _HOELDB_H

#include <stddef.h>
#include <stdbool.h>

#include "../simulatorapi/simapi/simapi/simdata.h"

typedef struct DBData DBData;
typedef struct DBField DBField;
typedef struct SessionFieldsData SessionFieldsData;
typedef struct SessionFields SessionFields;
typedef struct Session Session;
typedef unsigned char PDBTimeStamp[27];

struct DBField
{
    size_t offset;
    size_t size;
    int colnum;
    int type;
    char name[20];
};

//struct SessionFields
//{
//    DBField session_id;
//    DBField laps;
//};

typedef struct SessionRowData
{
    int session_id;
    int event_id;
    int event_type;
    //char track_time[5];
    //char session_name[50];
    //char start_time[6];
    //int duration_min;
    //int elapsed_ms;
    int stints;
    unsigned char sim[150];
    unsigned char track[150];
    unsigned char driver[150];
    unsigned char car[150];
    PDBTimeStamp start_time;
    //char weather[100];
    //double air_temp;
    //double road_temp;
    //double start_grip;
    //double current_grip;
    //int is_finished;
    //char finish_time[6];
    //char last_activity[6];
    //int http_port;

}
SessionRowData;

//struct SessionFieldsData
//{
//    int session_id;
//    int laps;
//};

typedef struct StintRowData
{
    int stint_id;
    int driver_id;
    int team_member_id;
    int session_id;
    int car_id;
    int game_car_id;
    int laps;
    int valid_laps;
    int best_lap_id;
    unsigned char tyre[50];
}
StintRowData;

typedef struct LapRowData
{
    int lap_id;
    int stint_id;
    int sector_1;
    int sector_2;
    int sector_3;
    double grip;
    unsigned char tyre[50];
    int time;
    int cuts;
    int crashes;
    int max_speed;
    int avg_speed;
    PDBTimeStamp finished_at;
    double f_tyre_temp;
    double r_tyre_temp;
    double f_tyre_wear;
    double r_tyre_wear;
    double f_tyre_press;
    double r_tyre_press;
    double f_brake_temp;
    double r_brake_temp;
}
LapRowData;

typedef struct SessionDbo
{
    int numrows;
    bool hasdata;

    DBField fields[9];
    SessionRowData* rows;
}
SessionDbo;

typedef struct StintDbo
{
    int numrows;
    bool hasdata;

    DBField fields[9];
    StintRowData* rows;
}
StintDbo;

typedef struct LapDbo
{
    int numrows;
    bool hasdata;

    DBField fields[21];
    LapRowData* rows;
}
LapDbo;

LapTime hoel_convert_to_simdata_laptime(int hoel_laptime);

int getLastInsertID(struct _h_connection* conn);

void get_row_results(struct _h_result result, DBField* fields, void* rows, size_t rowsize);

int getsessions(struct _h_connection* conn, const char* sessionname, SessionDbo* sess);

int getstints(struct _h_connection* conn, const char* sessionname, StintDbo* stint, int use_id);

int getlaps(struct _h_connection* conn, const char* sessionname, LapDbo* laps, int use_id);

int gettrack(struct _h_connection* conn, const char* trackname);

int getdriver(struct _h_connection* conn, const char* driver_name);

int getcar(struct _h_connection* conn, const char* carname);

int addtrackconfig(struct _h_connection* conn, int trackconfigid, const char* track, int length);

int adddriver(struct _h_connection* conn, int driverid, const char* drivername);

int addevent(struct _h_connection* conn, int track_config);

int addsession(struct _h_connection* conn, int eventid, int simid, int driverid, int carid, int sessiontype, int airtemp, int tracktemp, SimData* simdata);

int addstint(struct _h_connection* conn, int sessionid, int driverid, int carid, SimData* simdata);

int addstintlap(struct _h_connection* conn, int stintid, SimData* simdata);

int addcar(struct _h_connection* conn, int carid, const char* carname);

int addtelemetry(struct _h_connection* conn, int points, int stintid);

int closelap(struct _h_connection* conn, int lapid, int stintid, int sector1, int sector2, int sector3, int cuts, int crashes, double maxspeed, double avgspeed, SimData* simdata);

int closestint(struct _h_connection* conn, int stintid, int stintlaps, int validstintlaps);

int closesession(struct _h_connection* conn, int sessionid);
#endif
