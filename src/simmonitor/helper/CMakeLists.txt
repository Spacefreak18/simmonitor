set(helper_source_files
  parameters.c
  parameters.h
  dirhelper.c
  dirhelper.h
  confighelper.c
  confighelper.h
)

add_library(helper STATIC ${helper_source_files})
