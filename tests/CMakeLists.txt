include(Catch)

add_executable(entity_test main.cpp testcase.cpp)
target_link_libraries(entity_test entitypp)
set_target_properties(entity_test PROPERTIES COMPILE_FLAGS "-g" )
catch_discover_tests(entity_test)