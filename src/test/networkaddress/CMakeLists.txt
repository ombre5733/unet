set(test_SOURCES tst_networkaddress.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../networkaddress.cpp)
add_executable(tst_networkaddress ${test_SOURCES})
add_test(NetworkAddress tst_networkaddress)
#add_test(NAME PluginManager COMMAND tst_unetheader "-xunitxml")
