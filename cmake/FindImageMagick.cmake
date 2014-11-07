include(FindPkgConfig)
pkg_check_modules(IMAGEMAGICK Magick++ REQUIRED)
add_definitions(${IMAGEMAGICK_CFLAGS})
