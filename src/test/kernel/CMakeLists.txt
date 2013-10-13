set(test_SOURCES tst_kernel.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc
                 ../../networkaddress.cpp
                 ../../networkinterface.cpp
                 ../../routingtable.cpp)
add_executable(tst_kernel ${test_SOURCES})
add_test(Kernel tst_kernel)
