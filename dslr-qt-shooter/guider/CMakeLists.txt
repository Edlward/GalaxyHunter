add_library(dslr_guider STATIC linguider.cpp ${dslr_guider_HEADERS_MOC})
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_guider PARENT_SCOPE)
