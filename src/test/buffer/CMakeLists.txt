set(test_SOURCES tst_buffer.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../buffer.cpp)
add_executable(tst_buffer ${test_SOURCES})
add_test(Buffer tst_buffer)

set(test_SOURCES tst_bufferpool.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../buffer.cpp)
add_executable(tst_bufferpool ${test_SOURCES})
add_test(Buffer tst_bufferpool)

set(test_SOURCES tst_bufferqueue.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../buffer.cpp)
add_executable(tst_bufferqueue ${test_SOURCES})
add_test(Buffer tst_bufferqueue)
