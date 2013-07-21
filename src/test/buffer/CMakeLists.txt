set(test_SOURCES tst_buffer.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../buffer.cpp)
add_executable(tst_buffer ${test_SOURCES})
add_test(Buffer tst_buffer)
#add_test(NAME PluginManager COMMAND tst_unetheader "-xunitxml")
