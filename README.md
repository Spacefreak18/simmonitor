# SimMonitor
Customizable Simulator dashboards and telemetry data logger

## Features
- cli mode
- web ui
- ncurses ui
- customizable embedded UI
- records telemetry sampled 4 times a second to database, with data channels for steering, brake, acceleration, speed, and rpms

[![gilles1.png](https://i.postimg.cc/JhgrQB8c/gilles1.png)](https://postimg.cc/ns4fFrCC)
(ncurses interface ```simonitor play -u curses```)

[![multiple-plots-1.png](https://i.postimg.cc/pTPCzp83/multiple-plots-1.png)](https://postimg.cc/2qXnfj0x)

(sample gnuplot output of multiple laps telmetry comparison, accessed through ```simmonitor browse```)

## Suported Games
  - Using [SimSHMBridge](https://github.com/spacefreak18/simshmbridge)
    - Asseto Corsa (requires CrewChief plugin for best results)
    - Assetto Corsa Competizione
    - Project Cars 2
    - Automobilista 2

  - Using [scs-sdk-plugin](https://github.com/jackz314/scs-sdk-plugin/releases)
    - Euro Truck Simuator 2
    - American Truck Simulator

  - Using [rf2SharedMemoryMapPlugin_Wine](https://github.com/schlegp/rF2SharedMemoryMapPlugin_Wine)
    - RFactor2
    - LeMans Ultimate

## Dependencies
- argtable2
- libconfig
- ncurses
- microhttpd
- libxdg
- libuv (as a submodule)
- hoel
- lvgl (for configurable embedded displays, as a submodule)
- sdl2 (and sdl2_image, required by lvgl)
- tar
- postgresql, mariadb, sqlite3 (required to link to libhoel)
-- the rest are git submodules
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)
- [ctemplate](https://github.com/spacefreak18/ctemplate)
- [lvgl](https://github.com/spacefreak18/lvgl)
- [slog](https://github.com/kala13x/slog)
- [simshmbridge](https://github.com/spacefreak18/simshmbridge)
- [simapi](https://github.com/spacefreak18/simapi)

Arch Linux Dependencies
```
pacman -Syu libxdg-basedir argtable libconfig ncurses libmicrohttpd libuv hoel sdl2 sdl2_image libtar postgresql mariadb-libs sqlite3
```

## Building

This code depends on the shared memory data headers in the simapi [repo](https://github.com/spacefreak18/simapi). When pulling lastest if the submodule does not download run:
```
git submodule sync --recursive
git submodule update --init --recursive
```
Then to compile simply:
```
mkdir build; cd build
cmake ..
make
```

Once compiled copy conf files to ~/.config/simmonitor and data files to ~/.local/share/simmonitor.

## DB Setup
```
CREATE DATABASE simmonitor;
```
```
psql -d simmonitor -f data/simmonitor-psql.sql
```
for postgres 15+
```
GRANT pg_read_all_data TO USER;
GRANT pg_write_all_data TO USER;
```
for postgres 15 and below
```
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO USER;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO USER;
GRANT ALL PRIVILEGES ON ALL FUNCTIONS IN SCHEMA pbulic TO USER;
```

## Usage
### normal Second Monitor display with x gui
```
simmonitor play -u x
```
### normal Second Monitor display with web
```
simmonitor play -u web
```
Options are curses, web, x, cli, fb

### Run in CLI mode and capture telemetry
```
simmonitor play -u cli -m
```
### Browse previous sessions
```
simmonitor browse
```
Currently the browser is only in ncurses.

## Testing

### Static Analysis
```
    mkdir build; cd build
    make clean
    CFLAGS=-fanalyzer cmake ..
    make
```
### Valgrind
```
    cd build
    valgrind -v --leak-check=full --show-leak-kinds=all --suppressions=../.valgrindrc ./simmonitor play
```

## Join the Discussion
[Sim Racing Matrix Space](https://matrix.to/#/#simracing:matrix.org)

## ToDo
 - much, much more
