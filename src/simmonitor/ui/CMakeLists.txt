set(ui_source_files
  ncursesui.h
  ncursesui.c
  xgui.h
  xgui.c
  simfb.c
  simfb.h
  simwebui.c
  simwebui.h
  ../ctemplate/ctemplate.h
  ../ctemplate/ctemplate.c
)

set(FREETYPE_INCLUDE_DIR /usr/include/freetype2)
include_directories("." ${FREETYPE_INCLUDE_DIR})

add_library(ui STATIC ${ui_source_files})

target_compile_definitions(ui PRIVATE SIMMAP_ALL)
