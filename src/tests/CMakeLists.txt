add_definitions(-DBOOST_TEST_DYN_LINK)

add_definitions(-DTESTS_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(test_sequence testsequence.cpp)
target_link_libraries(test_sequence Qt5::Test)
add_test(NAME sequence COMMAND test_sequence)


#pkg_check_modules(LIBINDI libindi REQUIRED)
include_directories(${LIBINDI_INCLUDE_DIRS})
include_directories(/usr/include/libnova) # TODO: find_package?

add_executable(testindidouble testindidouble.cpp ${CMAKE_SOURCE_DIR}/src/telescope/indidouble.cpp)
target_link_libraries(testindidouble Qt5::Test ${INDI_CLIENT_LIBRARIES} ${LIBINDI_LIBRARIES} z pthread )
add_test(NAME indidouble COMMAND testindidouble)

add_executable(test_raw_conversion testrawconversion.cpp ${CMAKE_SOURCE_DIR}/src/imaging/file2image.cpp)
target_link_libraries(test_raw_conversion Qt5::Widgets Qt5::Test ${MAGICK_LDFLAGS})
add_test(NAME raw_conversion COMMAND test_raw_conversion)

#find_package(ROOT REQUIRED)
#include_directories(${ROOT_INCLUDE_DIR})
#add_executable(focusing_testing focusing_test_tool.cpp)
#target_link_libraries(focusing_testing Qt5::Widgets ${ROOT_LIBRARIES})
