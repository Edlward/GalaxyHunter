add_definitions(-DBOOST_TEST_DYN_LINK)

add_definitions(-DTESTS_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(test_sequence testsequence.cpp)
target_link_libraries(test_sequence Qt5::Test)
add_test(NAME sequence COMMAND test_sequence)

add_executable(test_raw_conversion testrawconversion.cpp ${CMAKE_SOURCE_DIR}/dslr-qt-shooter/imaging/file2image.cpp)
target_link_libraries(test_raw_conversion Qt5::Widgets Qt5::Test ${IMAGEMAGICK_LDFLAGS})
add_test(NAME raw_conversion COMMAND test_raw_conversion)