set(test_SOURCES tst_linklayeraddress.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc)
add_executable(tst_linklayeraddress ${test_SOURCES})
add_test(LinkLayerAddress tst_linklayeraddress)
