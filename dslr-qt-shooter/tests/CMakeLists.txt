add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable(test_sequence testsequence.cpp)
target_link_libraries(test_sequence ${Boost_LIBRARIES})
add_test(NAME sequence COMMAND test_sequence)