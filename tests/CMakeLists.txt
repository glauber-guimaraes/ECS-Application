include(Catch)

add_executable(entity_test main.cpp testcase.cpp)
target_link_libraries(entity_test entitypp)
catch_discover_tests(entity_test)