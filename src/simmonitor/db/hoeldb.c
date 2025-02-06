#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <hoel.h>
#include <jansson.h>

#include "hoeldb.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../helper/confighelper.h"
#include "../slog/src/slog.h"

LapTime hoel_convert_to_simdata_laptime(int hoel_laptime)
{
    LapTime l;
    l.minutes = hoel_laptime/60000;
    l.seconds = hoel_laptime/1000-(l.minutes*60);
    l.fraction = hoel_laptime-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

int getLastInsertID(struct _h_connection* conn)
{
    json_t* last_id = (h_last_insert_id(conn));
    int id = json_integer_value(last_id);
    json_decref(last_id);
    return id;
}

void get_row_results(struct _h_result result, DBField* fields, void* rows, size_t rowsize)
{
    int col, row, i;
    char buf[64];
    //sess->hasdata = true;
    //sess->rows = result.nb_rows;
    slogi("rows: %d, col: %d", result.nb_rows, result.nb_columns);
    char* aaa = (char*) rows;
    for (row = 0; row<result.nb_rows; row++)
    {

        char* aa = (char*)aaa + (rowsize* row);
        for (col=0; col<result.nb_columns; col++)
        {
            char* a = (char*) aa + fields[col].offset;
            switch(result.data[row][col].type)
            {
                case HOEL_COL_TYPE_INT:
                    int bb = ((struct _h_type_int*)result.data[row][col].t_data)->value;
                    *(int*) a = bb;
                    break;
                case HOEL_COL_TYPE_DOUBLE:
                    double cc = ((struct _h_type_double*)result.data[row][col].t_data)->value;
                    *(double*) a = cc;
                    break;
                case HOEL_COL_TYPE_TEXT:
                    char* ddd = ((struct _h_type_text*)result.data[row][col].t_data)->value;
                    memcpy(a, ddd, fields[col].size);
                    break;
                case HOEL_COL_TYPE_BLOB:
                    for (i=0; i<((struct _h_type_blob*)result.data[row][col].t_data)->length; i++)
                    {
                        printf("%c", *((char*)(((struct _h_type_blob*)result.data[row][col].t_data)->value+i)));
                        if (i%80 == 0 && i>0)
                        {
                            printf("\n");
                        }
                    }
                    break;
                case HOEL_COL_TYPE_DATE:
                    strftime(a, fields[col].size, "%Y-%m-%d %H:%M:%S", &((struct _h_type_datetime*)result.data[row][col].t_data)->value);
                    //strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &((struct _h_type_datetime *)result.data[row][col].t_data)->value);
                    printf("| %s ", buf);
                case HOEL_COL_TYPE_NULL:
                    printf("| [null] ");
                    break;
            }
        }
        printf("|\n");
    }
}

int getsessions(struct _h_connection* conn, const char* sessionname, SessionDbo* sess)
{
    struct _h_result result;
    struct _h_data* data;
    char* query;
    slogt("Performing query sessions");

    asprintf(&query, "select sessions.session_id, events.event_id, sessions.event_type, table1.stints, sims.sim_name, track_config.display_name, drivers.driver_name, cars.car_name, start_time "
            "FROM %s JOIN events ON sessions.event_id=events.event_id JOIN track_config ON events.track_config_id=track_config.track_config_id "
            "JOIN sims ON sessions.sim_id=sims.sim_id "
            "JOIN cars ON sessions.car_id=cars.car_id "
            "JOIN drivers ON sessions.driver_id=drivers.driver_id "
            "JOIN (Select session_id, COUNT(stint_id) AS stints FROM stints GROUP BY session_id) AS table1 ON table1.session_id=sessions.session_id "
            "ORDER BY sessions.session_id DESC LIMIT 25", "Sessions");

    int errcode = h_query_select(conn, query, &result);
    if (errcode == H_OK)
    {
        sess->rows = malloc(sizeof(SessionRowData) * result.nb_rows);
        get_row_results(result, sess->fields, sess->rows, sizeof(SessionRowData));
        slogi("%s", sess->rows[0].sim);
        h_clean_result(&result);
    }
    else
    {
        sloge("Error executing query %s returned %i", query, errcode);
        free(query);
        return -1;
    }
    free(query);

    return result.nb_rows;
}

int getstints(struct _h_connection* conn, const char* sessionname, StintDbo* stint, int use_id)
{
    struct _h_result result;
    struct _h_data* data;
    char* query = malloc(150 * sizeof(char));
    slogt("Performing query stints");


    sprintf(query, "select stint_id, driver_id, team_member_id, session_id, car_id, game_car_id, laps, valid_laps, best_lap_id FROM %s WHERE session_id=%i", "Stints", use_id);
    slogt("execute query %s", query);
    int errcode = h_query_select(conn, query, &result);
    if (errcode == H_OK)
    {
        stint->rows = malloc(sizeof(StintRowData) * result.nb_rows);
        get_row_results(result, stint->fields, stint->rows, sizeof(StintRowData));
        h_clean_result(&result);
    }
    else
    {
        sloge("Error executing query %s returned %i", query, errcode);
        free(query);
        return -1;
    }
    free(query);

    return result.nb_rows;
}

int getlaps(struct _h_connection* conn, const char* sessionname, LapDbo* laps, int use_id)
{
    struct _h_result result;
    struct _h_data* data;
    char* query = malloc(250 * sizeof(char));
    slogt("Performing query laps");

    sprintf(query, "select lap_id, stint_id, sector_1, sector_2, sector_3, grip, tyre, time, cuts, crashes, max_speed, avg_speed, finished_at FROM %s WHERE %s=%i", "Laps", "stint_id", use_id);
    int errcode = h_query_select(conn, query, &result);
    if (errcode == H_OK)
    {
        laps->rows = malloc(sizeof(LapRowData) * result.nb_rows);
        get_row_results(result, laps->fields, laps->rows, sizeof(LapRowData));
        h_clean_result(&result);
    }
    else
    {
        sloge("Error executing query %s returned %i", query, errcode);
        free(query);
        return -1;
    }
    free(query);

    return result.nb_rows;
}

int gettrack(struct _h_connection* conn, const char* trackname)
{

    json_t* j_result;
    char* where_clause = h_build_where_clause(conn, "config_name=%s AND track_name=%s", "default", trackname);
    json_t* j_query = json_pack("{sss[s]s{s{ssss}}}", "table", "track_config", "columns", "track_config_id", "where", " ", "operator", "raw",
                                "value", where_clause);

    char* qq;
    int res = h_select(conn, j_query, &j_result, &qq);

    // Deallocate j_query since it won't be needed anymore
    //json_decref(j_query);
    //h_free(where_clause);
    int track_config = -1;
    // Test query execution result
    if (res == H_OK)
    {
        // Print result
        //char* dump = json_dumps(j_result, JSON_INDENT(2));
        //slogi("json select result is\n%s", dump);
        int index1 = json_array_size(j_result);
        //if (index1 == 0)
        //{
        //    slogw("no config for this track");
        //}
        //else {
        json_t* jj = json_array_get(j_result, index1-1);
        track_config = json_integer_value(json_object_get(jj, "track_config_id"));
        //}
        // Deallocate data result

        //free(dump);
    }
    else
    {
        sloge("Error executing select query %s: %d", qq, res);
    }
    json_decref(j_result);
    free(qq);
    json_decref(j_query);
    h_free(where_clause);
    return track_config;
}

int getdriver(struct _h_connection* conn, const char* driver_name)
{
    json_t* j_result;
    char* where_clause = h_build_where_clause(conn, "driver_name=%s", driver_name);
    json_t* j_query = json_pack("{sss[s]s{s{ssss}}}","table", "drivers", "columns", "driver_id", "where", " ", "operator", "raw",
                                "value", where_clause);

    slogi("Looking for driver named %s", driver_name);
    char* qq;
    int res = h_select(conn, j_query, &j_result, &qq);
    //slogi("here your query: %s", qq);
    // Deallocate j_query since it won't be needed anymore
    int driver_id = -1;
    // Test query execution result
    if (res == H_OK)
    {
        // Print result
        char* dump = json_dumps(j_result, JSON_INDENT(2));
        slogt("json select result is\n%s", dump);
        int index1 = json_array_size(j_result);
        if (index1 == 0)
        {
            slogw("no driver by this name");
        }
        else
        {
            json_t* jj = json_array_get(j_result, index1-1);
            driver_id = json_integer_value(json_object_get(jj, "driver_id"));
        }
        // Deallocate data result
        free(dump);
    }
    else
    {
        sloge("Error executing select query %s: %d", qq, res);
    }
    json_decref(j_result);
    free(qq);
    json_decref(j_query);
    h_free(where_clause);
    return driver_id;
}

int getcar(struct _h_connection* conn, const char* carname)
{
    json_t* j_result;
    char* where_clause = h_build_where_clause(conn, "car_name=%s", carname);
    json_t* j_query = json_pack("{sss[s]s{s{ssss}}}","table", "cars", "columns", "car_id", "where", " ", "operator", "raw",
                                "value", where_clause);

    slogi("Looking for car named %s", carname);
    char* qq;
    int res = h_select(conn, j_query, &j_result, &qq);
    //slogi("here your query: %s", qq);
    // Deallocate j_query since it won't be needed anymore

    int car_id = -1;
    // Test query execution result
    if (res == H_OK)
    {
        // Print result
        char* dump = json_dumps(j_result, JSON_INDENT(2));
        slogt("json select result is\n%s", dump);
        int index1 = json_array_size(j_result);
        if (index1 == 0)
        {
            slogw("no car by this name");
        }
        else
        {
            json_t* jj = json_array_get(j_result, index1-1);
            car_id = json_integer_value(json_object_get(jj, "car_id"));
        }
        // Deallocate data result
        free(dump);
    }
    else
    {
        sloge("Error executing select query %s: %d", qq, res);
    }
    json_decref(j_result);
    free(qq);
    json_decref(j_query);
    h_free(where_clause);
    return car_id;
}


// inserts
int addtrackconfig(struct _h_connection* conn, int trackconfigid, const char* track, int length)
{
    //track_config_id | track_name | config_name | display_name | country | city | length
    if (trackconfigid > 0)
    {
        return trackconfigid;
    }
    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("track_config") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();
    json_object_set_new(values, "track_name", json_string(track));
    json_object_set_new(values, "config_name", json_string("default"));
    json_object_set_new(values, "display_name", json_string(track));
    json_object_set_new(values, "country", json_string("USA"));
    json_object_set_new(values, "city", json_string("USA"));
    json_object_set_new(values, "length", json_integer(length));

    json_array_append(json_arr, values);
    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("track config insert result %i", res);
        sloge("track config query %s", qq);
        return -1;
    }

    trackconfigid = getLastInsertID(conn);
    return trackconfigid;
}

int addsession(struct _h_connection* conn, int eventid, int simid, int driverid, int carid, int sessiontype, int airtemp, int tracktemp, SimData* simdata)
{

// session_id | event_id | event_type | track_time | session_name
// | start_time | duration_min | elapsed_ms | laps | weather |
// air_temp | road_temp | start_grip |
    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("sessions") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();
    json_object_set_new(values, "event_id", json_integer(eventid));
    json_object_set_new(values, "sim_id", json_integer(simid));
    json_object_set_new(values, "car_id", json_integer(carid));
    json_object_set_new(values, "driver_id", json_integer(driverid));
    json_object_set_new(values, "session_type", json_integer(sessiontype));
    json_object_set_new(values, "duration_min", json_integer(60));
    json_object_set_new(values, "session_name", json_string("default"));
    json_object_set_new(values, "air_temp", json_integer(airtemp));
    json_object_set_new(values, "road_temp", json_integer(tracktemp));
    json_object_set_new(values, "weather", json_string("Windy"));
    json_object_set_new(values, "http_port", json_integer(0));
    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("track config insert result %i", res);
        sloge("track config query %s", qq);
        return -1;
    }

    return getLastInsertID(conn);
}

int adddriver(struct _h_connection* conn, int driverid, const char* drivername)
{
    if (driverid > 0)
    {
        return driverid;
    }
    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("drivers") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();
    json_object_set_new(values, "driver_name", json_string(drivername));
    json_object_set_new(values, "prev_name", json_string(drivername));
    json_object_set_new(values, "steam64_id", json_integer(17));
    json_object_set_new(values, "country", json_string("USA"));
    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("driver insert result %i", res);
        sloge("driver insert query %s", qq);
        return -1;
    }


    driverid = getLastInsertID(conn);
    return driverid;
}


int addevent(struct _h_connection* conn, int track_config)
{

    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("events") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();
    json_object_set_new(values, "track_config_id", json_integer(track_config));
    json_object_set_new(values, "event_name", json_string("default"));
    //event_id | server_name | track_config_id | event_name | team_event | active | livery_preview | use_number | practice_duration | quali_duration | race_duration | race_duration_type | race_wait_time | race_extra_laps | reverse_grid_positions
    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("event insert result %i", res);
        sloge("event insert query %s", qq);
        return -1;
    }

    return getLastInsertID(conn);
}


int addstint(struct _h_connection* conn, int sessionid, int driverid, int carid, SimData* simdata)
{


    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("stints") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();

    json_object_set_new(values, "driver_id", json_integer(driverid));
    //team_member_id
    json_object_set_new(values, "session_id", json_integer(sessionid));
    json_object_set_new(values, "car_id", json_integer(carid));
    json_object_set_new(values, "game_car_id", json_integer(carid));
    json_object_set_new(values, "laps", json_integer(0));
    json_object_set_new(values, "valid_laps", json_integer(0));
    json_object_set_new(values, "tyre", json_string(simdata->tyrecompound) );
    // best_lap_id
    json_object_set_new(values, "is_finished", json_integer(0));

    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("stint insert result %i", res);
        sloge("stint insert query %s", qq);
        return -1;
    }

    return getLastInsertID(conn);
}

int addstintlap(struct _h_connection* conn, int stintid, SimData* simdata)
{

    slogt("adding stint lap");
// stint laps
// stint_lap_id | stint_id | sector_1 | sector_2 | sector_3 | grip | tyre | time | cuts | crashes
// max_speed | avg_speed | finished_at
    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("laps") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();

    json_object_set_new(values, "stint_id", json_integer(stintid));
    json_object_set_new(values, "tyre", json_string(simdata->tyrecompound) );
    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("stint lap insert result %i", res);
        sloge("stint lap insert query %s", qq);
        return -1;
    }

    return getLastInsertID(conn);
}

int addcar(struct _h_connection* conn, int carid, const char* carname)
{

    // car_id | display_name | car_name | manufacturer | car_class

    if (carid > 0)
    {
        return carid;
    }
    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("cars") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();
    json_object_set_new(values, "display_name", json_string(carname));
    json_object_set_new(values, "car_name", json_string(carname));
    json_object_set_new(values, "manufacturer", json_string("Unknown"));
    json_object_set_new(values, "car_class", json_string("Unknown"));
    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("car insert result %i", res);
        sloge("car insert query %s", qq);
        return -1;
    }

    return getLastInsertID(conn);
}

int addtelemetry(struct _h_connection* conn, int points, int stintid)
{

    slogt("adding telemetry record");

    json_t* root = json_object();
    json_t* json_arr = json_array();

    json_object_set_new( root, "table", json_string("telemetry") );
    json_object_set_new( root, "values", json_arr );

    json_t* values = json_object();
    json_object_set_new(values, "lap_id", json_integer(stintid));
    json_object_set_new(values, "points", json_integer(points));
    json_array_append(json_arr, values);

    char* qq;
    int res = h_insert(conn, root, &qq);
    json_decref(root);
    json_decref(values);

    if (res != 0)
    {
        sloge("telemetry insert result %i", res);
        sloge("telemetry insert query %s", qq);
        return -1;
    }

    return getLastInsertID(conn);
}

int closelap(struct _h_connection* conn, int lapid, int sector1, int sector2, int sector3, int cuts, int crashes, double maxspeed, double avgspeed, SimData* simdata)
{

// stint laps
// lap_id | stint_id | sector_1 | sector_2 | sector_3 | grip | tyre | time | cuts | crashes
// max_speed | avg_speed | finished_at

    slogt("closing previous lap ", lapid);

    int laptime = (simdata->lastlap.minutes * 60000) + (simdata->lastlap.seconds * 1000) + simdata->lastlap.fraction;
    char* query;

    asprintf(&query, "UPDATE %s SET %s=%i, %s=%i, %s=%i, %s=%i, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, %s=%f, finished_at=CURRENT_TIMESTAMP WHERE lap_id=%i;",
             "laps", "time", laptime, "sector_1", sector1, "sector_2", sector2, "sector_3", sector3,
             "right_front_tyre_temp", simdata->tyretemp[1],
             "right_rear_tyre_temp",  simdata->tyretemp[3],
             "right_front_tyre_wear", simdata->tyrewear[1],
             "right_rear_tyre_wear",  simdata->tyrewear[3],
             "right_front_tyre_press",simdata->tyrepressure[1],
             "right_rear_tyre_press", simdata->tyrepressure[3],
             "right_front_brake_temp",simdata->braketemp[1],
             "right_rear_brake_temp", simdata->braketemp[3],
             "left_front_tyre_temp", simdata->tyretemp[0],
             "left_rear_tyre_temp",  simdata->tyretemp[2],
             "left_front_tyre_wear", simdata->tyrewear[0],
             "left_rear_tyre_wear",  simdata->tyrewear[2],
             "left_front_tyre_press",simdata->tyrepressure[0],
             "left_rear_tyre_press", simdata->tyrepressure[2],
             "left_front_brake_temp",simdata->braketemp[0],
             "left_rear_brake_temp", simdata->braketemp[2],
             "max_speed", maxspeed,
             "avg_speed", avgspeed,
             lapid);

    int errcode = h_query_update(conn, query);
    if (errcode != H_OK)
    {
        sloge("Error executing query %s returned %i", query, errcode);
        free(query);
        return -1;
    }

    slogt("closed previous lap");
    free(query);
    return errcode;
}

int closesession(struct _h_connection* conn, int sessionid)
{

    slogt("closing previous session ", sessionid);
// best_lap_id

// session_id | event_id | event_type | track_time | session_name
// | start_time | duration_min | elapsed_ms | laps | weather |
// air_temp | road_temp | start_grip | end_grip | is_finished

    char* query;
    asprintf(&query, "UPDATE %s SET is_finished=1, finished_at=CURRENT_TIMESTAMP WHERE session_id=%i",
            "sessions", sessionid);

    int errcode = h_query_update(conn, query);
    if (errcode != H_OK)
    {
        sloge("Error executing query %s returned %i", query, errcode);
        free(query);
        return -1;
    }

    slogt("closed previous session");
    free(query);
    return errcode;
}


int closestint(struct _h_connection* conn, int stintid, int stintlaps, int validstintlaps)
{

// best_lap_id
    slogt("closing previous stint ", stintid);

    char* query;
    asprintf(&query, "UPDATE %s SET %s = %i, %s = %i, is_finished=1, finished_at=CURRENT_TIMESTAMP WHERE stint_id=%i",
            "stints", "laps", stintlaps, "valid_laps", validstintlaps, stintid);

    int errcode = h_query_update(conn, query);
    if (errcode != H_OK)
    {
        sloge("Error executing query %s returned %i", query, errcode);
        free(query);
        return -1;
    }

    slogt("closed previous stint");
    free(query);
    return errcode;
}
