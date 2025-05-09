
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <hoel.h>
#include <jansson.h>

#include "../db/hoeldb.h"
#include "telemetry.h"
#include "../slog/src/slog.h"

int telem_result(struct _h_result result, int doublefields, int intfields, int* intarrays, double* doublearrays)
{
    int col, row, i;
    char buf[64];
    slogt("rows: %d, col: %d", result.nb_rows, result.nb_columns);
    //int* intarrays;
    int points = 0;
    //int doublefields = 3;
    //int intfields = 3;
    //int* intarrays;
    //int* doublearrays;
    //int* intarrays = malloc((sizeof(int)*1736)*3);
    //double* doublearrays = malloc((sizeof(double)*1736)*3);
    int intarrayoffset = 0;
    int doublearrayoffset = 0;

    for (row = 0; row<result.nb_rows; row++)
    {
        for (col=0; col<result.nb_columns; col++)
        {

            slogt("col type %i", result.data[row][col].type);
            switch(result.data[row][col].type)
            {

                case HOEL_COL_TYPE_INT:
                    int cc = ((struct _h_type_int*)result.data[row][col].t_data)->value;
                    if (col == 1)
                    {
                        points = cc;
                    }
                    break;
                case HOEL_COL_TYPE_DOUBLE:
                    //intarrays = malloc((sizeof(int)*1736)*3);
                    break;
                case HOEL_COL_TYPE_TEXT:
                    // sqlite blobs are coming over as text
                    //slogi("| %s ", ((struct _h_type_text*)result.data[row][col].t_data)->value);
                    //break;
                case HOEL_COL_TYPE_BLOB:
                    int offset2 = 0;


                    int j = 0;
                    i = 0;
                    if (col < 5)
                    {
                        while (i<((struct _h_type_blob*)result.data[row][col].t_data)->length)
                        {
                            int k = 0;
                            char sss[10];
                            sss[0] = '0';
                            sss[1] = 'x';
                            sss[2] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+0+k);
                            sss[3] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+1+k);
                            sss[4] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+2+k);
                            sss[5] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+3+k);
                            sss[6] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+4+k);
                            sss[7] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+5+k);
                            sss[8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+6+k);
                            sss[9] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+7+k);
                            //long val;
                            uint32_t number = (uint32_t)strtol(sss, NULL, 16);
                            //uint32_t swapped = (uint32_t)__bswap_32(number);

                            //intarrays[j+intarrayoffset] = (uint32_t)__bswap_32(number);
                            intarrays[j+intarrayoffset] = (uint32_t)(number);
                            offset2 = offset2 + 8;
                            i+=8;
                            j++;
                        }
                        intarrayoffset += points;
                    }
                    else
                    {
                        while (i<((struct _h_type_blob*)result.data[row][col].t_data)->length)
                        {
                            int k = 0;
                            char sss[19];
                            sss[0] = '0';
                            sss[1] = 'x';
                            sss[2] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+0+k);
                            sss[3] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+1+k);
                            sss[4] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+2+k);
                            sss[5] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+3+k);
                            sss[6] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+4+k);
                            sss[7] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+5+k);
                            sss[8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+6+k);
                            sss[9] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+7+k);
                            sss[2+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+0+8+k);
                            sss[3+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+1+8+k);
                            sss[4+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+2+8+k);
                            sss[5+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+3+8+k);
                            sss[6+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+4+8+k);
                            sss[7+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+5+8+k);
                            sss[8+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+6+8+k);
                            sss[9+8] = *((char*)((struct _h_type_blob*)result.data[row][col].t_data)->value+offset2+7+8+k);
                            sss[18] = 0;
                            //long val;
                            //slogt("%s",sss);
                            unsigned long long number;
                            number = strtoull(sss, NULL, 16);
                            double d = *((double*)&number);
                            doublearrays[j+doublearrayoffset] = (double) d;
                            offset2 = offset2 + 16;
                            i+=16;
                            j++;
                        }
                        doublearrayoffset += points;

                    }
                    //snprintf( "blob value: %.*s", ((struct _h_type_blob *)result.data[row][col].t_data)->length, ((struct _h_type_blob *)result.data[row][col].t_data)->value);
                    //char* b = malloc(sizeof(int)*1736);
                    //for (i=0; i<((struct _h_type_blob *)result.data[row][col].t_data)->length; i++) {
                    //  //slogi("%c", *((char*)(((struct _h_type_blob *)result.data[row][col].t_data)->value+i)));
                    //  memcpy(&b[i], ((struct _h_type_blob *)result.data[row][col].t_data)->value+1*sizeof(char), sizeof(char));
                    //}
//          FILE *out = fopen("memory.bin", "wb");
//  if(out != NULL)
//  {
//    size_t to_go = sizeof(int)*1736;
//    while(to_go > 0)
//    {
//      const size_t wrote = fwrite(b, to_go, 1, out);
//      if(wrote == 0)
//        break;
//      to_go -= wrote;
//    }
//    fclose(out);
//  }
                    break;
                case HOEL_COL_TYPE_DATE:
                    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &((struct _h_type_datetime*)result.data[row][col].t_data)->value);
                    printf("| %s ", buf);
                case HOEL_COL_TYPE_NULL:
                    slogi("| [null] ");
                    break;
            }
        }


        printf("|\n");
    }

    return points;
}

int dumplapstofile(char* datadir, SessionDbo sessions, LapDbo lapsdb, int numlaps, int sessidx)
{

    slogt("dumping %d laps to temp file: stint: %d", numlaps, lapsdb.rows[0].stint_id);


    char* filename1 = "laps.out";
    char* datafile;

    asprintf(&datafile, "%s%s", datadir, filename1);
    FILE* out = fopen(datafile, "w");
    slogt("opend laps.out");
    fprintf(out, "%s at %s, %s\n", sessions.rows[sessidx].car, sessions.rows[sessidx].track, sessions.rows[sessidx].start_time);
    // easy to go back and add
    //fprintf(out, "weather: %s, air temp: %d, track temp: %d\n", sessions.rows[sessidx].weather);
    //fprintf(out, "tyre compound: %s\n", lapsdb.rows[0].tyre);
    fprintf(out, "%s %s  %s  %s  %s\n", "Lap", "Sector1", "Sector2", "Sector3", "Time");

    for (int i=0; i<numlaps; i++)
    {
        char* s1;
        char* s2;
        char* s3;
        char* l;

        LapTime ss1 = hoel_convert_to_simdata_laptime( lapsdb.rows[i].sector_1 );
        asprintf(&s1, "%d:%02d:%03d", ss1.minutes, ss1.seconds, ss1.fraction);

        LapTime ss2 = hoel_convert_to_simdata_laptime( lapsdb.rows[i].sector_2 );
        asprintf(&s2, "%d:%02d:%03d", ss2.minutes, ss2.seconds, ss2.fraction);

        LapTime ss3 = hoel_convert_to_simdata_laptime( lapsdb.rows[i].sector_3 );
        asprintf(&s3, "%d:%02d:%03d", ss3.minutes, ss3.seconds, ss3.fraction);

        LapTime sl = hoel_convert_to_simdata_laptime( lapsdb.rows[i].time );
        asprintf(&l, "%d:%02d:%03d", sl.minutes, sl.seconds, sl.fraction);

        if(i < 9)
        {
            fprintf(out, "%i   %s %s %s %s\n", i+1, s1, s2, s3, l);
        }
        else
        {
            fprintf(out, "%i  %s %s %s %s\n", i+1, s1, s2, s3, l);
        }


        free(s1);
        free(s2);
        free(s3);
        free(l);
    }
    fclose(out);


    return 0;
}

int dumptelemetrytofile(struct _h_connection* conn, char* datadir, int lap1id, int lap2id)
{

    slogt("dumping telemetry to temp file: lap1id: %d lap2id: %d", lap1id, lap2id);

    int points = 0;
    int intfields = 3;
    int doublefields = 3;

    struct _h_result result;
    struct _h_data* data;
    char* query = malloc(150 * sizeof(char));
    sprintf(query, "SELECT lap_id, points FROM %s WHERE %s=%i", "telemetry", "lap_id", lap1id);
    if (h_query_select(conn, query, &result) == H_OK)
    {
        //laps->rows = malloc(sizeof(LapRowData) * result.nb_rows);
        //get_row_results(result, laps->fields, laps->rows, sizeof(LapRowData));
        points = telem_result(result, 3, 3, NULL, NULL);
        //get_stint_result(result, stint);
        h_clean_result(&result);
    }
    else
    {
        printf("Error executing query\n");
    }
    free(query);


    uint32_t* intarrays1 = malloc((sizeof(uint32_t))*points* intfields);
    double* doublearrays1 = malloc((sizeof(double))*points* doublefields);
    uint32_t* intarrays2 = malloc((sizeof(uint32_t))*points* intfields);
    double* doublearrays2 = malloc((sizeof(double))*points* doublefields);

    struct _h_result result1;
    struct _h_data* data1;
    char* query1;
    //asprintf(&query1, "SELECT lap_id, points, speed, rpms, gear, brake, accel, steer FROM %s WHERE %s=%i", "telemetry", "lap_id", lap1id);
    asprintf(&query1, "SELECT lap_id, points, hex(speed), hex(rpms), hex(gear), hex(brake), hex(accel), hex(steer) FROM %s WHERE %s=%i", "telemetry", "lap_id", lap1id);
    if (h_query_select(conn, query1, &result1) == H_OK)
    {
        //laps->rows = malloc(sizeof(LapRowData) * result.nb_rows);
        //get_row_results(result, laps->fields, laps->rows, sizeof(LapRowData));
        points = telem_result(result1, intfields, doublefields, intarrays1, doublearrays1);
        //get_stint_result(result, stint);
        h_clean_result(&result1);
    }
    else
    {
        printf("Error executing query\n");
    }
    free(query1);

    struct _h_result result2;
    struct _h_data* data2;
    char* query2;
    //asprintf(&query2, "SELECT lap_id, points, speed, rpms, gear, brake, accel, steer FROM %s WHERE %s=%i", "telemetry", "lap_id", lap2id);
    asprintf(&query2, "SELECT lap_id, points, hex(speed), hex(rpms), hex(gear), hex(brake), hex(accel), hex(steer) FROM %s WHERE %s=%i", "telemetry", "lap_id", lap2id);
    if (h_query_select(conn, query2, &result2) == H_OK)
    {
        //laps->rows = malloc(sizeof(LapRowData) * result.nb_rows);
        //get_row_results(result, laps->fields, laps->rows, sizeof(LapRowData));
        points = telem_result(result2, intfields, doublefields, intarrays2, doublearrays2);
        //get_stint_result(result, stint);
        h_clean_result(&result2);
    }
    else
    {
        printf("Error executing query\n");
    }
    free(query2);

    char* filename1 = "data.out";
    size_t strsize = strlen(datadir) + strlen(filename1) + 1;
    char* datafile = malloc(strsize);

    snprintf(datafile, strsize, "%s%s", datadir, filename1);
    slogt("dumping %i points to file %s", points, datafile);
    FILE* out = fopen(datafile, "w");
    fprintf(out, "%s %s %s %s %s %s %s %s %s %s %s %s %s\n", "point", "speed1", "rpms1", "gear1", "brake1", "accel1", "steer1", "speed2", "rpms2", "gear2", "brake2", "accel2", "steer2" );

    // this could be made configurable at some point, but i think the graphs are too noisey with the shifts into neutral for each shift
    bool hideneutral = true;
    int lastgear1 = 0;
    int lastgear2 = 0;
    for (int i=0; i<points; i++)
    {
        int gear1 = intarrays1[i+(points*2)];
        int gear2 = intarrays2[i+(points*2)];
        double steer1 = doublearrays1[i+(points*2)];
        double steer2 = doublearrays2[i+(points*2)];
        if (steer1 == 0.00 || steer2 == 0.00)
        {
            continue;
        }
        if (hideneutral == true)
        {
            if( gear1 == 1)
            {
                gear1 = lastgear1;
            }
            if( gear2 == 1)
            {
                gear2 = lastgear2;
            }
        }
        fprintf(out, "%i %i %i %i %f %f %f", i+1, intarrays1[i], intarrays1[i+points], gear1-1, doublearrays1[i], doublearrays1[i+points], doublearrays1[i+(points*2)]);
        // make sure there is an extra space at the beginning of this
        fprintf(out, " %i %i %i %f %f %f\n", intarrays2[i], intarrays2[i+points], gear2-1, doublearrays2[i], doublearrays2[i+points], doublearrays2[i+(points*2)]);
        lastgear1 = gear1;
        lastgear2 = gear2;
    }
    fclose(out);

    free(intarrays1);
    free(intarrays2);
    free(doublearrays1);
    free(doublearrays2);
    free(datafile);

    return 1;
}

int updatetelemetry(struct _h_connection* conn, int telemid, int samples, size_t size, const char* column, void* data)
{
    int totalsize = samples * size;
    char output[(totalsize * 2) + 1];
    char* ppp = &output[0];
    unsigned char* p = data;
    int i = 0;
    int k = size - 1;
    while (i<totalsize)
    {
        k = size - 1;
        while (k >= 0)
        {
            ppp += sprintf(ppp, "%02hhX", p[i+k]);
            k--;
        }
        i = i + size;
    }

    //print_bytes(output, size);
    //char* query;
    //slogt("data %s", &output);
    char* query;
    //char* query = malloc((sizeof(char)*71)+(sizeof(column))+(size*2)+1);
    //sprintf(query, "UPDATE telemetry SET %s = decode('%s', 'hex') WHERE telemetry_id = %i", column, &output, telemid);
    asprintf(&query, "UPDATE telemetry SET %s = unhex('%s') WHERE telemetry_id = %i", column, &output, telemid);
    slogt("query: %s", query);
    int res1 = h_query_update(conn, query);
    //int res1 = h_insert(conn, j_query, NULL);
    slogt("got res %i", res1);
    free(query);

    return res1;
}

int updatetelemetrydata(struct _h_connection* conn, int tracksamples, int telemid, int lapid,
                        int* speeddata, int* rpmdata, int* geardata,
                        double* steerdata, double* acceldata, double* brakedata)
{
    int b = 0;
    b = updatetelemetry(conn, telemid, tracksamples, sizeof(double), "steer", steerdata);
    b = updatetelemetry(conn, telemid, tracksamples, sizeof(double), "accel", acceldata);
    b = updatetelemetry(conn, telemid, tracksamples, sizeof(double), "brake", brakedata);
    b = updatetelemetry(conn, telemid, tracksamples, sizeof(int), "rpms", rpmdata);
    b = updatetelemetry(conn, telemid, tracksamples, sizeof(int), "gear", geardata);
    b = updatetelemetry(conn, telemid, tracksamples, sizeof(int), "speed", speeddata);

    return b;
}
