cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread -Wall")

set(CMAKE_BUILD_TYPE "debug")

find_package(Boost REQUIRED)

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions("-DWEOS_USER_CONFIG=\"${CMAKE_CURRENT_SOURCE_DIR}/user_config.hpp\"")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(.)
include_directories(../3rdparty)

add_executable(unet
#               ../neighborcache.cpp
               ../networkaddress.cpp
               ../networkinterface.cpp
               ../routingtable.cpp
               ../protocol/simplemessageprotocol.cpp
               ../main.cpp)
target_link_libraries(unet ${Boost_LIBRARIES})

add_subdirectory(buffer)
add_subdirectory(kernel)
add_subdirectory(linklayeraddress)
add_subdirectory(neighbor)
add_subdirectory(networkinterface)
add_subdirectory(networkaddress)
add_subdirectory(networkprotocol)
#add_subdirectory(timeoutlist)
#add_subdirectory(unetheader)
