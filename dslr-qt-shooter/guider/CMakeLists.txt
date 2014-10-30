QT4_WRAP_CPP(dslr-guider_HEADERS_MOC linguider.h)
add_library(dslr-guider STATIC linguider.cpp ${dslr-guider_HEADERS_MOC})
set(dslr-static_LIBS ${dslr-static_LIBS} dslr-guider PARENT_SCOPE)
