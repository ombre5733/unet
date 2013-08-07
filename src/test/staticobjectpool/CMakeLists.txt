set(test_SOURCES tst_staticobjectpool.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc)
add_executable(tst_staticobjectpool ${test_SOURCES})
add_test(StaticObjectPool tst_staticobjectpool)
#add_test(NAME PluginManager COMMAND tst_unetheader "-xunitxml")
