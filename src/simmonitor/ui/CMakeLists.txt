
set(ui_source_files
  ncursesui.h
  ncursesui.c
  simwebui.c
  simwebui.h
  lvgui.c
  lvgui.h
  lvhelper.h
  lvhelper.c
  lv_conf.h
  ../ctemplate/ctemplate.h
  ../ctemplate/ctemplate.c
)

file(COPY lv_conf.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lvgl)

set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
set(LV_CONF_BUILD_DISABLE_DEMOS ON)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON)
add_subdirectory("lvgl")

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
pkg_check_modules(LIBDRM REQUIRED libdrm)
find_package(SDL2_image REQUIRED)

target_include_directories(lvgl PUBLIC ${LIBDRM_INCLUDE_DIRS})
target_include_directories(lvgl PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(lvgl PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})

add_library(ui STATIC ${ui_source_files})

target_link_libraries(ui lvgl SDL2::SDL2 X11 ${LIBDRM_LIBRARIES})
