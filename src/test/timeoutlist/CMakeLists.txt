set(test_SOURCES tst_timeoutlist.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../neighborcache.cpp)
add_executable(tst_timeoutlist ${test_SOURCES})
add_test(TimeoutList tst_timeoutlist)
#add_test(NAME PluginManager COMMAND tst_unetheader "-xunitxml")
