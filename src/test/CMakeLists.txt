cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread -Wall")

set(CMAKE_BUILD_TYPE "debug")

find_package(Boost REQUIRED)

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(unet
               ../buffer.cpp
               ../kernel.cpp
               ../neighborcache.cpp
               ../networkaddress.cpp
               ../networkinterface.cpp
               ../routingtable.cpp
               ../main.cpp)
target_link_libraries(unet ${Boost_LIBRARIES})

add_subdirectory(buffer)
add_subdirectory(networkaddress)
add_subdirectory(unetheader)
