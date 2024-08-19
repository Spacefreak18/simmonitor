#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>
#include <termios.h>
#include <hoel.h>
#include <jansson.h>

#include <byteswap.h>

#include "gameloop.h"
#include "../db/hoeldb.h"
#include "../telemetry/telemetry.h"

#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../helper/dirhelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"

#define DEFAULT_UPDATE_RATE      60
#define DATA_UPDATE_RATE         4


#define SESSIONS_SCREEN     1
#define STINTS_SCREEN       2
#define LAPS_SCREEN         3


WINDOW* bwin1;
WINDOW* bwin2;
WINDOW* bwin3;
WINDOW* bwin4;

int bwinx, bwiny;

char blanks[1000];


LapTime hoel_convert_to_simdata_laptime(int hoel_laptime)
{
    LapTime l;
    l.minutes = hoel_laptime/60000;
    l.seconds = hoel_laptime/1000-(l.minutes*60);
    l.fraction = hoel_laptime-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

void add_line()
{
    wprintw(bwin1, "\n");
    for (int j = 0;  j < bwiny-1;  ++j)
    {
        waddch(bwin1, ACS_HLINE);
    }
}

void set_spaces(int spaces)
{
    blanks[spaces] = '\0';
}

void b_handle_winch(int sig)
{
    endwin();

    refresh();
    clear();
    getmaxyx(stdscr, bwinx, bwiny);
    refresh();
}


int b_curses_init()
{
    memset(blanks,' ',sizeof(blanks));
    initscr();
    start_color();

    init_pair(1,COLOR_GREEN,0);
    init_pair(2,COLOR_YELLOW,0);
    init_pair(5,COLOR_BLACK,COLOR_YELLOW);
    init_pair(3,COLOR_MAGENTA,0);
    init_pair(4,COLOR_WHITE,0);

    getmaxyx(stdscr, bwinx, bwiny);
    slogt("windowx %i, windowy %i", bwinx, bwiny);
    bwin1 = newwin(bwinx,bwiny,0,0);

    wbkgd(bwin1,COLOR_PAIR(1));
    wbkgd(bwin2,COLOR_PAIR(1));
    wbkgd(bwin3,COLOR_PAIR(1));
    wbkgd(bwin4,COLOR_PAIR(1));

    signal(SIGWINCH, b_handle_winch);
    curs_set(0);
    cbreak();
    noecho();

    box(bwin1, 0, 0);
}



void* browseloop(SMSettings* sms, char* datadir)
{

    struct _h_result result;
    struct _h_connection* conn;
    conn = h_connect_pgsql(sms->db_conn);

    if (conn == NULL)
    {
        slogf("Unable to connect to configured SimMonitor database. Are the parameters in the config correct? Is the user allowed to access from this address?");
        sms->dberr = E_FAILED_DB_CONN;
        return 0;
    }

    slogt("Starting analyzer");
    b_curses_init();

    timeout(DEFAULT_UPDATE_RATE);

    SessionDbo sess;
    DBField sessid;
    sessid.type = HOEL_COL_TYPE_INT;
    sessid.offset = 0;
    sessid.colnum = 0;
    DBField laps;
    laps.type = HOEL_COL_TYPE_INT;
    laps.offset = sizeof(int);
    laps.offset = offsetof(SessionRowData, laps);
    laps.colnum = 1;
    DBField eventid;
    eventid.type = HOEL_COL_TYPE_INT;
    eventid.offset = offsetof(SessionRowData, event_id);
    DBField eventtype;
    eventtype.type = HOEL_COL_TYPE_INT;
    eventtype.offset = offsetof(SessionRowData, event_type);
    DBField track;
    track.type = HOEL_COL_TYPE_TEXT;
    track.offset = offsetof(SessionRowData, track);
    track.size = sizeof(unsigned char)*150;
    DBField car;
    car.type = HOEL_COL_TYPE_TEXT;
    car.offset = offsetof(SessionRowData, car);
    car.size = sizeof(unsigned char)*150;
    DBField starttime;
    starttime.type = HOEL_COL_TYPE_DATE;
    starttime.offset = offsetof(SessionRowData, start_time);
    starttime.size = sizeof(PDBTimeStamp);

    //SessionFields sf;
    sess.fields[0] = sessid;
    sess.fields[1] = eventid;
    sess.fields[2] = eventtype;
    sess.fields[3] = laps;
    sess.fields[4] = track;
    sess.fields[5] = car;
    sess.fields[6] = starttime;
    //sf.session_id = sessid;
    //sf.laps = laps;

    //sess.fields = sf;



    int action = 0;
    int selection = 1;
    int lastselection = 1;
    int selection1 = 0;
    int selection2 = 0;
    int sessions = 0;
    int lapsresults = 0;
    int curresults = 0;
    int stintsid = 0;

    StintDbo stints;
    DBField stintid;
    stintid.type = HOEL_COL_TYPE_INT;
    stintid.offset = 0;
    DBField driverid;
    driverid.type = HOEL_COL_TYPE_INT;
    driverid.offset = offsetof(StintRowData, driver_id);
    DBField teammemberid;
    teammemberid.type = HOEL_COL_TYPE_INT;
    teammemberid.offset = offsetof(StintRowData, team_member_id);
    DBField sessionidstint;
    sessionidstint.type = HOEL_COL_TYPE_INT;
    sessionidstint.offset = offsetof(StintRowData, session_id);
    DBField carid;
    carid.type = HOEL_COL_TYPE_INT;
    carid.offset = offsetof(StintRowData, car_id);
    DBField gamecarid;
    gamecarid.type = HOEL_COL_TYPE_INT;
    gamecarid.offset = offsetof(StintRowData, game_car_id);
    DBField stintlaps;
    stintlaps.type = HOEL_COL_TYPE_INT;
    stintlaps.offset = offsetof(StintRowData, laps);
    DBField validlaps;
    validlaps.type = HOEL_COL_TYPE_INT;
    validlaps.offset = offsetof(StintRowData, valid_laps);
    DBField bestlapid;
    bestlapid.type = HOEL_COL_TYPE_INT;
    bestlapid.offset = offsetof(StintRowData, best_lap_id);

    stints.fields[0] = stintid;
    stints.fields[1] = driverid;
    stints.fields[2] = teammemberid;
    stints.fields[3] = sessionidstint;
    stints.fields[4] = carid;
    stints.fields[5] = gamecarid;
    stints.fields[6] = stintlaps;
    stints.fields[7] = validlaps;
    stints.fields[8] = bestlapid;

    LapDbo lapsdb;
    DBField lapsdbid;
    lapsdbid.type = HOEL_COL_TYPE_INT;
    lapsdbid.offset = 0;
    DBField lapsdbstintid;
    lapsdbstintid.type = HOEL_COL_TYPE_INT;
    lapsdbstintid.offset = offsetof(LapRowData, stint_id);
    DBField sector_1;
    sector_1.type = HOEL_COL_TYPE_INT;
    sector_1.offset = offsetof(LapRowData, sector_1);
    DBField sector_2;
    sector_2.type = HOEL_COL_TYPE_INT;
    sector_2.offset = offsetof(LapRowData, sector_2);
    DBField sector_3;
    sector_3.type = HOEL_COL_TYPE_INT;
    sector_3.offset = offsetof(LapRowData, sector_3);
    DBField grip;
    grip.type = HOEL_COL_TYPE_DOUBLE;
    grip.offset = offsetof(LapRowData, grip);
    DBField tyrec;
    tyrec.type = HOEL_COL_TYPE_TEXT;
    tyrec.offset = offsetof(LapRowData, tyre);
    tyrec.size = sizeof(unsigned char)*50;
    DBField time;
    time.type = HOEL_COL_TYPE_INT;
    time.offset = offsetof(LapRowData, time);
    DBField cuts;
    cuts.type = HOEL_COL_TYPE_INT;
    cuts.offset = offsetof(LapRowData, cuts);
    DBField carcrashes;
    carcrashes.type = HOEL_COL_TYPE_INT;
    carcrashes.offset = offsetof(LapRowData, crashes);
    DBField maxspeed;
    maxspeed.type = HOEL_COL_TYPE_DOUBLE;
    maxspeed.offset = offsetof(LapRowData, max_speed);
    DBField avgspeed;
    avgspeed.type = HOEL_COL_TYPE_DOUBLE;
    avgspeed.offset = offsetof(LapRowData, avg_speed);
    DBField f_tyre_temp;
    f_tyre_temp.type = HOEL_COL_TYPE_DOUBLE;
    f_tyre_temp.offset = offsetof(LapRowData, f_tyre_temp);
    DBField r_tyre_temp;
    r_tyre_temp.type = HOEL_COL_TYPE_DOUBLE;
    r_tyre_temp.offset = offsetof(LapRowData, r_tyre_temp);
    DBField f_tyre_wear;
    f_tyre_wear.type = HOEL_COL_TYPE_DOUBLE;
    f_tyre_wear.offset = offsetof(LapRowData, f_tyre_wear);
    DBField r_tyre_wear;
    r_tyre_wear.type = HOEL_COL_TYPE_DOUBLE;
    r_tyre_wear.offset = offsetof(LapRowData, r_tyre_wear);
    DBField f_tyre_press;
    f_tyre_press.type = HOEL_COL_TYPE_DOUBLE;
    f_tyre_press.offset = offsetof(LapRowData, f_tyre_press);
    DBField r_tyre_press;
    r_tyre_press.type = HOEL_COL_TYPE_DOUBLE;
    r_tyre_press.offset = offsetof(LapRowData, r_tyre_press);
    DBField f_brake_temp;
    f_brake_temp.type = HOEL_COL_TYPE_DOUBLE;
    f_brake_temp.offset = offsetof(LapRowData, f_brake_temp);
    DBField r_brake_temp;
    r_brake_temp.type = HOEL_COL_TYPE_DOUBLE;
    r_brake_temp.offset = offsetof(LapRowData, r_brake_temp);
    DBField lapsdbfinishedat;
    lapsdbfinishedat.type = HOEL_COL_TYPE_DATE;
    lapsdbfinishedat.offset = offsetof(LapRowData, finished_at);
    lapsdbfinishedat.size = sizeof(PDBTimeStamp);

    lapsdb.fields[0] = lapsdbid;
    lapsdb.fields[1] = lapsdbstintid;
    lapsdb.fields[2] = sector_1;
    lapsdb.fields[3] = sector_2;
    lapsdb.fields[4] = sector_3;
    lapsdb.fields[5] = grip;
    lapsdb.fields[6] = tyrec;
    lapsdb.fields[7] = time;
    lapsdb.fields[8] = cuts;
    lapsdb.fields[9] = carcrashes;
    lapsdb.fields[10] = maxspeed;
    lapsdb.fields[11] = avgspeed;
    lapsdb.fields[12] = lapsdbfinishedat;
    //lapsdb.fields[13] = f_tyre_temp;
    //lapsdb.fields[14] = r_tyre_temp;
    //lapsdb.fields[15] = f_tyre_wear;
    //lapsdb.fields[16] = r_tyre_wear;
    //lapsdb.fields[17] = f_tyre_press;
    //lapsdb.fields[18] = r_tyre_press;
    //lapsdb.fields[19] = f_brake_temp;
    //lapsdb.fields[20] = r_brake_temp;

    //slogt("sessions has %i rows", sess.numrows);

    action = 2;


    int go = true;
    char lastsimstatus = false;

    int screen = SESSIONS_SCREEN;
    char ch;
    box(bwin1, 0, 0);
    wrefresh(bwin1);
    int stint_useid = 0;
    int lap_useid = 0;

    char laptimechar1[10];
    char laptimechar2[10];

    while (go == true)
    {

        if (lastselection != selection)
        {
            action = 1;
            lastselection = selection;
        }

        if (action == 2)
        {
            slogt("going to perform an action");
            int err = E_NO_ERROR;
            sessions = getsessions(conn, "Sessions", &sess);
            if (sessions < 0)
            {
                go = false;
            }
            else
            {
                curresults = sessions;
            }
        }

        if (action == 3)
        {
            slogt("going to perform an action");

            int err = E_NO_ERROR;
            stintsid = getstints(conn, "Stints", &stints, stint_useid);
            curresults = stintsid;
            if (stintsid < 0)
            {
                go = false;
            }
            else
            {
                curresults = stintsid;
            }
        }
        if (action == 4)
        {
            slogt("going to perform an action");

            int err = E_NO_ERROR;
            lapsresults = getlaps(conn, "laps", &lapsdb, lap_useid);
            slogt("laps query executed");
            curresults = lapsresults;
            if (lapsresults < 0)
            {
                go = false;
            }
            else
            {
                curresults = lapsresults;
            }
        }

        if (action > 0)
        {
            wclear(bwin1);

            switch(screen)
            {
                case SESSIONS_SCREEN:


                    for(int i=0; i<sessions+1; i++)
                    {
                        int displaycolumns = 6;
                        int width1 = displaycolumns + 1;
                        int width2 = displaycolumns * 2;

                        if (i == 0)
                        {

                            wattrset(bwin1, COLOR_PAIR(2));
                            wattron(bwin1, A_BOLD);
                            mvwaddnstr(bwin1, 2, bwiny/2, "Sessions", -1);
                            mvwaddnstr(bwin1, 3, bwiny/width2, "idx", -1);
                            mvwaddnstr(bwin1, 3, bwiny/width1+bwiny/width2, "name", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*2)+bwiny/width2, "track", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*3)+bwiny/width2, "car", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*4)+bwiny/width2, "stints", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*5)+bwiny/width2, "time", -1);
                            mvwhline(bwin1, 4, 0, 0, bwiny);
                            wattroff(bwin1, A_BOLD);
                            wattrset(bwin1, COLOR_PAIR(1));
                        }
                        else
                        {

                            if ( i == selection )
                            {
                                wattrset(bwin1, COLOR_PAIR(5));
                                stint_useid = sess.rows[i-1].session_id;
                                mvwaddnstr(bwin1, 4+i, bwiny/width2, " * ", 3);

                            }
                            else
                            {
                                mvwaddnstr(bwin1, 4+i, bwiny/6, "   ", 3);
                            }
                            char idchar[4];
                            snprintf(idchar, 4, "%i", sess.rows[i-1].session_id);
                            char lapschar[4];
                            snprintf(lapschar, 4, "%i", sess.rows[i-1].laps);

                            mvwaddnstr(bwin1, 4+i, bwiny/width2+2, idchar, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+bwiny/width1, "my session name", -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*2), sess.rows[i-1].track, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*3), sess.rows[i-1].car, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*4), lapschar, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*5), sess.rows[i-1].start_time, -1);
                        }
                        wattrset(bwin1, COLOR_PAIR(1));
                    }
                    break;
                case STINTS_SCREEN:
                    if (stintsid == 0)
                    {
                        break;
                    }
                    for(int i=0; i<stintsid+1; i++)
                    {
                        if (i == 0)
                        {
                            wattrset(bwin1, COLOR_PAIR(2));
                            wattron(bwin1, A_BOLD);
                            mvwaddnstr(bwin1, 2, bwiny/2, "Stints", -1);
                            mvwaddnstr(bwin1, 3, bwiny/6, "idx", 3);
                            mvwaddnstr(bwin1, 3, bwiny/4+bwiny/6, "name", 4);
                            mvwaddnstr(bwin1, 3, ((bwiny/4)*2)+bwiny/6, "laps", 4);
                            mvwhline(bwin1, 4, 0, 0, bwiny);
                            wattroff(bwin1, A_BOLD);
                            wattrset(bwin1, COLOR_PAIR(1));
                        }
                        else
                        {

                            if ( i == selection )
                            {
                                wattrset(bwin1, COLOR_PAIR(5));
                                lap_useid = stints.rows[i-1].stint_id;
                                mvwaddnstr(bwin1, 4+i, bwiny/6, " * ", 3);
                            }
                            else
                            {
                                mvwaddnstr(bwin1, 4+i, bwiny/6, "   ", 3);
                            }
                            char idchar[4];
                            snprintf(idchar, 4, "%i", stints.rows[i-1].stint_id);
                            char lapschar[4];
                            snprintf(lapschar, 4, "%i", stints.rows[i-1].laps);

                            mvwaddnstr(bwin1, 4+i, bwiny/6+2, idchar, 4);
                            mvwaddnstr(bwin1, 4+i, bwiny/6+bwiny/4, "my session name", 15);
                            mvwaddnstr(bwin1, 4+i, bwiny/6+((bwiny/4)*2), lapschar, 4);
                        }
                        wattrset(bwin1, COLOR_PAIR(1));
                    }
                    break;
                case LAPS_SCREEN:
                    if (lapsresults == 0)
                    {
                        break;
                    }

                    for(int i=0; i<lapsresults+1; i++)
                    {

                        int displaycolumns = 5;
                        int width1 = displaycolumns + 1;
                        int width2 = displaycolumns * 2;
                        if (i == 0)
                        {
                            wattrset(bwin1, COLOR_PAIR(2));
                            wattron(bwin1, A_BOLD);
                            mvwaddnstr(bwin1, 2, bwiny/2, "Laps", -1);
                            mvwaddnstr(bwin1, 3, bwiny/width2, "idx", -1);
                            mvwaddnstr(bwin1, 3, bwiny/width1+bwiny/width2, "name", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*2)+bwiny/width2, "tyre", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*3)+bwiny/width2, "laptime", -1);
                            mvwaddnstr(bwin1, 3, ((bwiny/width1)*4)+bwiny/width2, "maxspeed", -1);
                            mvwhline(bwin1, 4, 0, 0, bwiny);
                            wattroff(bwin1, A_BOLD);
                            wattrset(bwin1, COLOR_PAIR(1));

                        }
                        else
                        {

                            //stint_useid = lapsdb.rows[i-1].rows;
                            if ( lapsdb.rows[i-1].lap_id == selection1 )
                            {
                                mvwaddnstr(bwin1, 4+i, bwiny/7 - 4, " 1 ", 3);
                                LapTime l = hoel_convert_to_simdata_laptime( lapsdb.rows[i-1].time);
                                snprintf(laptimechar1, 10, "%d:%02d:%03d", l.minutes, l.seconds, l.fraction);
                            }
                            else
                                if ( lapsdb.rows[i-1].lap_id == selection2 )
                                {
                                    mvwaddnstr(bwin1, 4+i, bwiny/7 - 4, " 2 ", 3);
                                    LapTime l = hoel_convert_to_simdata_laptime( lapsdb.rows[i-1].time);
                                    snprintf(laptimechar2, 10, "%d:%02d:%03d", l.minutes, l.seconds, l.fraction);
                                }
                            if ( i == selection )
                            {
                                wattrset(bwin1, COLOR_PAIR(5));
                                mvwaddnstr(bwin1, 4+i, bwiny/7, " * ", 3);
                            }
                            else
                            {
                                mvwaddnstr(bwin1, 4+i, bwiny/7, "   ", 3);

                            }
                            char idchar[4];
                            snprintf(idchar, 4, "%i", lapsdb.rows[i-1].lap_id);
                            char maxspeedchar[4];
                            snprintf(maxspeedchar, 4, "%f", lapsdb.rows[i-1].max_speed);
                            LapTime l = hoel_convert_to_simdata_laptime( lapsdb.rows[i-1].time);
                            char laptimechar[10];
                            snprintf(laptimechar, 10, "%d:%02d:%03d", l.minutes, l.seconds, l.fraction);

                            mvwaddnstr(bwin1, 4+i, bwiny/width2+2, idchar, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+bwiny/width1, "my session name", -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*2), lapsdb.rows[i-1].tyre, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*3), laptimechar, -1);
                            mvwaddnstr(bwin1, 4+i, bwiny/width2+((bwiny/width1)*4), maxspeedchar, -1);
                        }
                        wattrset(bwin1, COLOR_PAIR(1));

                    }
                    break;
            }
            action = 0;
        }

        box(bwin1, 0, 0);
        wrefresh(bwin1);

        scanf("%c", &ch);
        if(ch == 'q')
        {
            go = false;
        }
        if (ch == 'b')
        {
            switch(screen)
            {

                case STINTS_SCREEN:
                    action = 2;
                    screen = SESSIONS_SCREEN;


                    break;
                case LAPS_SCREEN:
                    action = 3;
                    screen = STINTS_SCREEN;
                    break;
            }
            selection = 1;
            lastselection = 1;
        }
        if (ch == 'e')
        {
            switch(screen)
            {

                case SESSIONS_SCREEN:
                    action = 3;
                    screen = STINTS_SCREEN;


                    break;
                case STINTS_SCREEN:
                    action = 4;
                    screen = LAPS_SCREEN;
                    break;
            }

            selection = 1;
            lastselection = 1;
        }
        if (ch == 'g')
        {
            //selection1 = 363;
            //selection2 = 362;
            if (sms->gnuplotfound == 1)
            {
                if (selection1 > 0 && selection2 > 0)
                {
                    dumptelemetrytofile(conn, datadir, selection1, selection2);

                    slogi("finished dumping data");
                    size_t strsize = strlen(datadir) + strlen(sms->gnuplot_file_str) + 1;
                    char* plotfile = malloc(strsize);
                    snprintf(plotfile, strsize, "%s%s", datadir, sms->gnuplot_file_str);
                    static char* argv1[]= {"gnuplot", "-p", "-c", "plotfile.gp", "hold", "hold", NULL};
                    argv1[3] = plotfile;
                    argv1[4] = laptimechar1;
                    argv1[5] = laptimechar2;
                    slogi("Using gnu plot file %s", plotfile);
                    if(!fork())
                    {
                        execv(sms->gnuplot_bin_str, argv1);
                    }
                }
            }
            else
            {
                sloge("No gnuplot binary found or specified");
            }
            action = 4;
        }
        if (ch == 'B' || ch == 'l')
        {
            selection++;
            if (selection > curresults)
            {
                selection = curresults;
            }
        }
        if (ch == 'A' || ch == 'k')
        {
            selection--;
            if (selection <= 1)
            {
                selection = 1;
            }
        }
        if (ch == '1')
        {
            selection1 = lapsdb.rows[selection-1].lap_id;
            action = 1;
        }
        if (ch == '2')
        {
            selection2 = lapsdb.rows[selection-1].lap_id;
            action = 1;
        }


    }



    wrefresh(bwin1);
    delwin(bwin1);
    endwin();

    h_close_db(conn);
    h_clean_connection(conn);

}

