set(gameloop_source_files
  gameloop.c
  gameloop.h
  browseloop.c
  browseloop.h
  loopdata.h
)

add_library(gameloop STATIC ${gameloop_source_files})
