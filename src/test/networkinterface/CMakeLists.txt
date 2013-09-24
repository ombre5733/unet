set(test_SOURCES tst_networkinterface.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../networkinterface.cpp)
add_executable(tst_networkinterface ${test_SOURCES})
add_test(Buffer tst_networkinterface)
