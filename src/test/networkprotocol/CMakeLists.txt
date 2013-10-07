set(test_SOURCES tst_networkprotocol.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc)
add_executable(tst_networkprotocol ${test_SOURCES})
add_test(NetworkProtocol tst_networkprotocol)
