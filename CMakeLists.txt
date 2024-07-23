cmake_minimum_required(VERSION 3.14)

project(OrderCache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
file(GLOB SOURCES src/*.cpp)
file(GLOB TESTS tests/*.cpp)

# TODO: fix me later
# possible check if CONDA_PREFIX is set
# maybe I'll use conan

find_library(GTEST_LIB NAMES gtest gtest_main HINTS $ENV{CONDA_PREFIX}/lib REQUIRED)
find_package(Threads REQUIRED)

add_library(OrderCacheLib STATIC ${SOURCES})
add_executable(OrderCacheTest ${TESTS})

include_directories(OrderCacheTest PRIVATE include $ENV{CONDA_PREFIX}/include)
target_link_libraries(OrderCacheTest PRIVATE OrderCacheLib Threads::Threads ${GTEST_LIB})