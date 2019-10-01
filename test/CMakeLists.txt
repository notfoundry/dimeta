set(test_files
        test_event_scheduling.cpp
        test_preprocessing.cpp)

add_executable(dimeta_test ${test_files})
target_link_libraries(dimeta_test PUBLIC dimeta)
target_compile_options(dimeta_test PRIVATE -ftemplate-backtrace-limit=0)

enable_testing()
add_test(NAME dimeta_test COMMAND dimeta_test)