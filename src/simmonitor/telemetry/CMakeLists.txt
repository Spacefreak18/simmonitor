set(telemetry_source_files
  telemetry.h
  telemetry.c
  telemmonitor.c
  telemmonitor.h
  ../ctemplate/ctemplate.h
  ../ctemplate/ctemplate.c
)

add_library(telemetry STATIC ${telemetry_source_files})

target_compile_definitions(telemetry PRIVATE SIMMAP_ALL)
