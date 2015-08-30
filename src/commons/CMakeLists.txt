configure_file(version.h.in version.h)
add_library(dsl_shooter_commons STATIC
  logmessage.cpp 
  shootersettings.cpp
)