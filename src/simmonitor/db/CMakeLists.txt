set(db_source_files
  hoeldb.h
  hoeldb.c
)

add_library(db STATIC ${db_source_files})

target_compile_definitions(db PRIVATE SIMMAP_ALL)
