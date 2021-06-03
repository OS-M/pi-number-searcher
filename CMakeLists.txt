cmake_minimum_required(VERSION 3.17)
project(pi)

include_directories($ENV{Boost_DIR}/include/boost-1_74/)

set(CMAKE_CXX_STANDARD 20)

add_executable(pi main.cpp)