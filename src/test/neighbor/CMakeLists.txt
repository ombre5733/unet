set(test_SOURCES tst_neighbor.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc)
add_executable(tst_neighbor ${test_SOURCES})
add_test(Neighbor tst_neighbor)

set(test_SOURCES tst_neighborcache.cpp
                 ../gtest/gtest-all.cc ../gtest/gtest_main.cc ../../networkinterface.cpp)
add_executable(tst_neighborcache ${test_SOURCES})
add_test(NeighborCache tst_neighborcache)
