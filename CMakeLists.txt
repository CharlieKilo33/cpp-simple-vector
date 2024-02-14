cmake_minimum_required(VERSION 3.26)
project(Practicum)

set(CMAKE_CXX_STANDARD 17)

add_executable(main simple-vector/main.cpp simple-vector/simple_vector.h simple-vector/array_ptr.h)