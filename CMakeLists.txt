cmake_minimum_required(VERSION 3.17)
project(pi)

find_package(Threads REQUIRED)

add_compile_options(-pthread)

set(CMAKE_CXX_STANDARD 20)

add_executable(pi main.cpp)

target_link_libraries(pi PRIVATE Threads::Threads)