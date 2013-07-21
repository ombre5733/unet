set(test_SOURCES tst_unetheader.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc)
add_executable(tst_unetheader ${test_SOURCES})
add_test(UnetHeader tst_unetheader)
#add_test(NAME PluginManager COMMAND tst_unetheader "-xunitxml")
