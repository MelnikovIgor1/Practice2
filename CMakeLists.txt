cmake_minimum_required(VERSION 3.16)
project(Practice2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(out main.cpp)

find_package( Threads )
find_package( GTest REQUIRED )
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(Test Tests.cpp test_main.cpp)
target_link_libraries(Test ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

