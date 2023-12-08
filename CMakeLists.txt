cmake_minimum_required(VERSION 3.10)
project(mvc_lab_2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-g -O3 -march=armv8.1-a+simd -mtune=cortex-a57 -ftree-vectorize")

add_executable(mvc_lab_2 main.cpp bmp_io.cpp)
