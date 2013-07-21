cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(unet
               ../buffer.cpp
               ../networkaddress.cpp
               ../networkinterface.cpp
               ../unetmanager.cpp
               ../main.cpp)

add_subdirectory(buffer)
add_subdirectory(networkaddress)
add_subdirectory(unetheader)
