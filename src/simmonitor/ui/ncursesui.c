#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <uv.h>

#include "../gameloop/loopdata.h"
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../helper/dirhelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/src/slog.h"


char datestring[30];

WINDOW* win1;
WINDOW* win2;
WINDOW* win3;
WINDOW* win4;

int winx, winy;

int win23y, win23x;


void handle_winch(int sig)
{
    endwin();

    refresh();
    clear();
    getmaxyx(stdscr, winx, winy);
    win23y = winy/3;
    win23x = winx/3;
    win1 = newwin(winx,winy,0,0);
    win2 = newwin(win23x,win23y,1,win23y-1);
    win3 = newwin(win23x,win23y,1,win23y*2-1);
    win4 = newwin(winx-win23x-2,winy-win23y,win23x+1,win23y-1);
    refresh();
}

void rectangle(WINDOW* win, int y1, int x1, int y2, int x2)
{
    mvwhline(win, y1, x1, 0, x2-x1);
    mvwhline(win, y2, x1, 0, x2-x1);
    mvwvline(win, y1, x1, 0, y2-y1);
    mvwvline(win, y1, x2, 0, y2-y1);
    mvwaddch(win, y1, x1, ACS_ULCORNER);
    mvwaddch(win, y2, x1, ACS_LLCORNER);
    mvwaddch(win, y1, x2, ACS_URCORNER);
    mvwaddch(win, y2, x2, ACS_LRCORNER);
}

int curses_init()
{
    initscr();
    start_color();

    init_pair(1,COLOR_GREEN,0);
    init_pair(2,COLOR_YELLOW,0);
    init_pair(3,COLOR_MAGENTA,0);
    init_pair(4,COLOR_WHITE,0);
    init_pair(5,COLOR_GREEN,COLOR_GREEN);
    init_pair(6,COLOR_YELLOW,COLOR_YELLOW);
    init_pair(7,COLOR_RED,COLOR_RED);
    init_pair(8,COLOR_WHITE,COLOR_BLACK);

    getmaxyx(stdscr, winx, winy);
    win1 = newwin(winx,winy,0,0);
    win23y = winy/3;
    win23x = winx/3;
    win2 = newwin(win23x,win23y,1,win23y-1);
    win3 = newwin(win23x,win23y,1,win23y*2-1);
    win4 = newwin(winx-win23x-2,winy-win23y,win23x+1,win23y-1);

    wbkgd(win1,COLOR_PAIR(1));
    wbkgd(win2,COLOR_PAIR(1));
    wbkgd(win3,COLOR_PAIR(1));
    wbkgd(win4,COLOR_PAIR(1));


    signal(SIGWINCH, handle_winch);
    cbreak();
    noecho();
    curs_set(0);


    box(win1, 0, 0);
    box(win2, 0, 0);
    box(win3, 0, 0);
    box(win4, 0, 0);
}

char* removeSpacesFromStr(char* string)
{
    int non_space_count = 0;

    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ')
        {
            string[non_space_count] = string[i];
            non_space_count++;
        }
    }

    string[non_space_count] = '\0';
    return string;
}

void cursescallback(uv_timer_t* handle)
{

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;


    slogt("curses callback");
    wclear(win1);
    wclear(win2);
    wclear(win3);
    wclear(win4);
    slogt("initial clear");
    {
        // window 1 car diagnostics

        int row = 2;
        int col1 = 3;
        int col2 = 7;
        char* speed;
        wbkgd(win1,COLOR_PAIR(1));
        wattrset(win1, COLOR_PAIR(1));
        asprintf(&speed, "%i", simdata->velocity);
        mvwaddnstr(win1, row, col1, "Speed:", -1);
        wattrset(win1, COLOR_PAIR(2));
        mvwaddnstr(win1, row, col1+col2, speed, -1);
        row++;

        char* rpm;
        wbkgd(win1,COLOR_PAIR(1));
        wattrset(win1, COLOR_PAIR(1));
        asprintf(&rpm, "%i", simdata->rpms);
        mvwaddnstr(win1, row, col1, "RPM:", -1);
        wattrset(win1, COLOR_PAIR(2));
        mvwaddnstr(win1, row, col1+col2, rpm, -1);
        row++;

        char* gear;
        wbkgd(win1,COLOR_PAIR(1));
        wattrset(win1, COLOR_PAIR(1));
        asprintf(&gear, "%i", simdata->gear);
        mvwaddnstr(win1, row, col1, "Gear:", -1);
        wattrset(win1, COLOR_PAIR(2));
        mvwaddnstr(win1, row, col1+col2, gear, -1);
        row++;

        char* gas;
        wattrset(win1, COLOR_PAIR(1));
        asprintf(&gas, "%f", simdata->gas);
        mvwaddnstr(win1, row, col1, "Gas:", -1);
        wattrset(win1, COLOR_PAIR(2));
        mvwaddnstr(win1, row, col1+col2, gas, -1);
        row++;

        char* brake;
        wattrset(win1, COLOR_PAIR(1));
        asprintf(&brake, "%f", simdata->brake);
        mvwaddnstr(win1, row, col1, "Brake:", -1);
        wattrset(win1, COLOR_PAIR(2));
        mvwaddnstr(win1, row, col1+col2, brake, -1);
        row++;

        char* fuel;
        wattrset(win1, COLOR_PAIR(1));
        asprintf(&fuel, "%f", simdata->fuel);
        mvwaddnstr(win1, row, col1, "Fuel: ", -1);
        wattrset(win1, COLOR_PAIR(2));
        mvwaddnstr(win1, row, col1+col2, fuel, -1);
        row++;
        row+=3;

        free(speed);
        free(rpm);
        free(gear);
        free(gas);
        free(brake);
        free(fuel);

        //flag
        wattrset(win1, COLOR_PAIR(simdata->courseflag+5));
        mvwhline(win1, 4, 40, 0, 20);
        mvwhline(win1, 5, 40, 0, 20);
        mvwhline(win1, 6, 40, 0, 20);
        mvwhline(win1, 7, 40, 0, 20);
        mvwhline(win1, 8, 40, 0, 20);
        wattrset(win1, COLOR_PAIR(1));

        // setup tyre and brake data strings
        char* braketemp0;
        char* braketemp1;
        char* braketemp2;
        char* braketemp3;
        asprintf(&braketemp0, "%.0f", simdata->braketemp[0]);
        asprintf(&braketemp1, "%.0f", simdata->braketemp[1]);
        asprintf(&braketemp2, "%.0f", simdata->braketemp[2]);
        asprintf(&braketemp3, "%.0f", simdata->braketemp[3]);

        char* tyretemp0;
        char* tyretemp1;
        char* tyretemp2;
        char* tyretemp3;
        asprintf(&tyretemp0, "%.0f", simdata->tyretemp[0]);
        asprintf(&tyretemp1, "%.0f", simdata->tyretemp[1]);
        asprintf(&tyretemp2, "%.0f", simdata->tyretemp[2]);
        asprintf(&tyretemp3, "%.0f", simdata->tyretemp[3]);

        char* tyrepressure0;
        char* tyrepressure1;
        char* tyrepressure2;
        char* tyrepressure3;
        asprintf(&tyrepressure0, "%.0f", simdata->tyrepressure[0]);
        asprintf(&tyrepressure1, "%.0f", simdata->tyrepressure[1]);
        asprintf(&tyrepressure2, "%.0f", simdata->tyrepressure[2]);
        asprintf(&tyrepressure3, "%.0f", simdata->tyrepressure[3]);

        char* tyrewear0;
        char* tyrewear1;
        char* tyrewear2;
        char* tyrewear3;
        asprintf(&tyrewear0, "%.0f", simdata->tyrewear[0]);
        asprintf(&tyrewear1, "%.0f", simdata->tyrewear[1]);
        asprintf(&tyrewear2, "%.0f", simdata->tyrewear[2]);
        asprintf(&tyrewear3, "%.0f", simdata->tyrewear[3]);

        {
            int row = 11;
            int col1 = 2;
            int col2 = 12;
            int col3 = 8;
            int col4 = 12;
            // front brakes

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "braketemp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, braketemp0, -1);
            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "braketemp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, braketemp1, -1);
            row++;


            // front tyres
            col2=7;
            col3= 16;
            col4=7;

            row++;
            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "temp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, tyretemp0, -1);

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "temp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, tyretemp1, -1);
            row++;

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "pres:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, tyrepressure0, -1);

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "pres:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, tyrepressure1, -1);
            row++;

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "wear:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, tyrewear0, -1);

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "wear:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, tyrewear1, -1);
            row++;


            // rear tyres

            row+=3;
            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "temp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, tyretemp2, -1);

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "temp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, tyretemp3, -1);
            row++;

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "pres:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, tyrepressure2, -1);

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "pres:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, tyrepressure3, -1);
            row++;

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "wear:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, tyrewear2, -1);

            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "wear:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, tyrewear3, -1);
            row++;

            // rear brakes
            row++;
            col2 = 12;
            col3 = 8;
            col4 = 12;
            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1, "braketemp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2, braketemp2, -1);
            wattrset(win1, COLOR_PAIR(1));
            mvwaddnstr(win1, row, col1+col2+col3, "braketemp:", -1);
            wattrset(win1, COLOR_PAIR(2));
            mvwaddnstr(win1, row, col1+col2+col3+col4, braketemp3, -1);
            row++;

            // draw boxes to represent the tyres
            wattrset(win1, COLOR_PAIR(1));
            rectangle(win1, 12, 13, 16, 17);  // left front
            rectangle(win1, 12, 19, 16, 23); // right front
            rectangle(win1, 18, 13, 22, 17);  // left rear
            rectangle(win1, 18, 19, 22, 23); // right rear



        }

        free(braketemp0);
        free(braketemp1);
        free(braketemp2);
        free(braketemp3);
        free(tyretemp0);
        free(tyretemp1);
        free(tyretemp2);
        free(tyretemp3);
        free(tyrepressure0);
        free(tyrepressure1);
        free(tyrepressure2);
        free(tyrepressure3);
        free(tyrewear0);
        free(tyrewear1);
        free(tyrewear2);
        free(tyrewear3);
    }
    slogt("initial diagnostics");
    {
        // window 2 session info

        int row = 1;
        int col1gap = 3;
        int col2gap = 14;

        char* airtemp;
        wbkgd(win2, COLOR_PAIR(1));
        wattrset(win2, COLOR_PAIR(1));
        asprintf(&airtemp, "%.0f", simdata->airtemp);
        mvwaddnstr(win2, row, col1gap, "Air Temp:", -1);
        wattrset(win2, COLOR_PAIR(2));
        mvwaddnstr(win2, row, col1gap+col2gap, airtemp, -1);
        row++;

        char* airdensity;
        wbkgd(win2, COLOR_PAIR(1));
        wattrset(win2, COLOR_PAIR(1));
        asprintf(&airdensity, "%.0f", simdata->airdensity);
        mvwaddnstr(win2, row, col1gap, "Humidity:", -1);
        wattrset(win2, COLOR_PAIR(2));
        mvwaddnstr(win2, row, col1gap+col2gap, airdensity, -1);
        row++;

        char* tracktemp;
        wbkgd(win2, COLOR_PAIR(1));
        wattrset(win2, COLOR_PAIR(1));
        asprintf(&tracktemp, "%.0f", simdata->tracktemp);
        mvwaddnstr(win2, row, col1gap, "Track Temp:", -1);
        wattrset(win2, COLOR_PAIR(2));
        mvwaddnstr(win2, row, col1gap+col2gap, tracktemp, -1);
        row++;

        char* numlaps;
        wbkgd(win2, COLOR_PAIR(1));
        wattrset(win2, COLOR_PAIR(1));
        asprintf(&numlaps, "%i", simdata->numlaps);
        mvwaddnstr(win2, row, col1gap, "Laps:", -1);
        wattrset(win2, COLOR_PAIR(2));
        mvwaddnstr(win2, row, col1gap+col2gap, numlaps, -1);
        row++;

        char* timeleft;
        wbkgd(win2, COLOR_PAIR(1));
        wattrset(win2, COLOR_PAIR(1));
        mvwaddnstr(win2, row, col1gap, "Session Time:", -1);
        asprintf(&timeleft, "%02d:%02d:%02d\n", simdata->sessiontime.hours, simdata->sessiontime.minutes, simdata->sessiontime.seconds);
        wattrset(win2, COLOR_PAIR(2));
        mvwaddnstr(win2, row, col1gap+col2gap, timeleft, -1);
        row++;


        wattrset(win2, COLOR_PAIR(1));

        free(airtemp);
        free(airdensity);
        free(tracktemp);
        free(numlaps);
        free(timeleft);
    }
    slogt("session info");

    {
        // window 3 basic timing and scoring

        int row = 1;
        int col1gap = 3;
        int col2gap = 14;

        char* car;
        char* track;
        char* driver;
        char* lap;
        char* position;
        char* lastlap;
        char* bestlap;

        wbkgd(win3, COLOR_PAIR(1));
        wattrset(win3, COLOR_PAIR(1));
        asprintf(&car, "%s", simdata->car);
        mvwaddnstr(win3, row, col1gap, "Car:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, car, -1);
        row++;

        wbkgd(win3, COLOR_PAIR(1));
        wattrset(win3, COLOR_PAIR(1));
        asprintf(&track, "%s", simdata->track);
        mvwaddnstr(win3, row, col1gap, "Track:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, track, -1);
        row++;

        wbkgd(win3, COLOR_PAIR(1));
        wattrset(win3, COLOR_PAIR(1));
        asprintf(&driver, "%s", simdata->driver);
        mvwaddnstr(win3, row, col1gap, "Driver:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, driver, -1);
        row++;

        wbkgd(win3, COLOR_PAIR(1));
        wattrset(win3, COLOR_PAIR(1));
        asprintf(&lap, "%i", simdata->lap);
        mvwaddnstr(win3, row, col1gap, "Lap:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, lap, -1);
        row++;

        wbkgd(win3, COLOR_PAIR(1));
        wattrset(win3, COLOR_PAIR(1));
        asprintf(&position, "%i", simdata->position);
        mvwaddnstr(win3, row, col1gap, "Position:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, position, -1);
        row++;

        wattrset(win3, COLOR_PAIR(1));
        asprintf(&lastlap, "%d:%02d:%03d\n", simdata->lastlap.minutes, simdata->lastlap.seconds, simdata->lastlap.fraction);
        mvwaddnstr(win3, row, col1gap, "Last Lap:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, lastlap, -1);
        row++;

        wattrset(win3, COLOR_PAIR(1));
        asprintf(&bestlap, "%d:%02d:%03d\n", simdata->bestlap.minutes, simdata->bestlap.seconds, simdata->bestlap.fraction);
        mvwaddnstr(win3, row, col1gap, "Best Lap:", -1);
        wattrset(win3, COLOR_PAIR(2));
        mvwaddnstr(win3, row, col1gap+col2gap, bestlap, -1);

        wattrset(win3, COLOR_PAIR(1));

        free(car);
        free(track);
        free(driver);
        free(lap);
        free(position);
        free(lastlap);
        free(bestlap);
    }
    slogt("basic timing and scoring");

    {
        // window 4 live standings timing and scoring

        int row = 1;
        wattrset(win4, COLOR_PAIR(4));
        mvwaddnstr(win4, row, 3, "P", -1);
        mvwaddnstr(win4, row, 10, "Driver", -1);
        mvwaddnstr(win4, row, 35, "Car", -1);
        mvwaddnstr(win4, row, 48, "Laps", -1);
        mvwaddnstr(win4, row, 58, "Last", -1);
        mvwaddnstr(win4, row, 71, "Best", -1);
        mvwaddnstr(win4, row, 82, "Pit", -1);
        row++;

        // figuring out how many and which entrants to display while respecting term size
        // this will take some work and testing
        int thisx, thisy;
        getmaxyx(win4, thisx, thisy);
        int maxdispcars = thisy/5;
        int displaycars = maxdispcars;
        if (displaycars > MAXCARS)
        {
            displaycars = MAXCARS;
        }
        if (simdata->numcars < displaycars)
        {
            displaycars = simdata->numcars;
        }
        for(int i=0; i<displaycars; i++)
        {
            int ihold = i;
            for(i=0; i<displaycars; i++)
            {
                if((ihold+1)==simdata->cars[i].pos)
                {
                    break;
                }
            }

            wattrset(win4, COLOR_PAIR(2));
            int maxstrlen = 20; // can i make this depend on screen width
            char* pos;
            char* car;
            char* driver;
            char* lap;

            asprintf(&pos, "%02d", simdata->cars[i].pos);
            asprintf(&driver, "%-*.*s", maxstrlen, maxstrlen, simdata->cars[i].driver);
            asprintf(&car, "%-*.*s", maxstrlen, maxstrlen, simdata->cars[i].car);
            asprintf(&lap, "%i", simdata->cars[i].lap);

            char* clastlap;
            asprintf(&clastlap, "%d:%02d:%03d\n", simdata->cars[i].lastlap.minutes, simdata->cars[i].lastlap.seconds, simdata->cars[i].lastlap.fraction);
            char* cbestlap;
            asprintf(&cbestlap, "%d:%02d:%03d\n", simdata->cars[i].bestlap.minutes, simdata->cars[i].bestlap.seconds, simdata->cars[i].bestlap.fraction);

            mvwaddnstr(win4, row, 2, pos, -1);
            mvwaddnstr(win4, row, 7, driver, -1);
            mvwaddnstr(win4, row, 27, car, -1);
            mvwaddnstr(win4, row, 50, lap, -1);
            mvwaddnstr(win4, row, 56, clastlap, -1);
            mvwaddnstr(win4, row, 69, cbestlap, -1);

            char* pitstatus;
            if(simdata->cars[i].inpitlane == 0 && simdata->cars[i].inpit == 0)
            {
                wattrset(win4, COLOR_PAIR(1));
                asprintf(&pitstatus, "%s", "ontrack");
            }
            else
            {
                if(simdata->cars[i].inpit > 0)
                {
                    asprintf(&pitstatus, "%s", "pit");
                }
                else
                {
                    asprintf(&pitstatus, "%s", "pitlane");
                }
            }
            mvwaddnstr(win4, row, 80, pitstatus, -1);
            row++;

            i = ihold;

            free(pos);
            free(car);
            free(driver);
            free(lap);
            free(clastlap);
            free(cbestlap);
            //free(pitstatus);
        }
        wattrset(win4, COLOR_PAIR(1));
    }

    box(win1, 0, 0);
    box(win2, 0, 0);
    box(win3, 0, 0);
    box(win4, 0, 0);

    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
    wrefresh(win4);

    //    if (getch() == 'q')
    //    {
    //        go = false;
    //    }
    //}

    if (f->uion == false)
    {
        wrefresh(win4);
        delwin(win4);
        endwin();

        wrefresh(win3);
        delwin(win3);
        endwin();

        wrefresh(win2);
        delwin(win2);
        endwin();

        wrefresh(win1);
        delwin(win1);
        endwin();
        uv_timer_stop(handle);
    }
}
