set(telemetry_source_files
  telemetry.h
  telemetry.c
  telemmonitor.c
  telemmonitor.h
  ../ctemplate/ctemplate.h
  ../ctemplate/ctemplate.c
)
set(YALP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(YALP_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(YALP_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(yalp)

add_library(telemetry STATIC ${telemetry_source_files})
target_link_libraries(telemetry PRIVATE yalp::yalp)
