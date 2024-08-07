cmake_minimum_required(VERSION 3.14)

project(OrderCache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
file(GLOB SOURCES src/*.cpp)
file(GLOB TESTS tests/*.cpp)

find_library(GTEST_LIB NAMES gtest gtest_main HINTS $ENV{EXTERN_LIBS}/lib REQUIRED)
find_package(Threads REQUIRED)

add_library(OrderCacheLib STATIC ${SOURCES})
add_executable(OrderCacheTest ${TESTS})

include_directories(OrderCacheTest PRIVATE include $ENV{EXTERN_LIBS}/include)
target_link_libraries(OrderCacheTest PRIVATE OrderCacheLib Threads::Threads ${GTEST_LIB})