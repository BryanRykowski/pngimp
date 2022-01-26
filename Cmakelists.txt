cmake_minimum_required(VERSION 3.12)

project(pngimp)

include_directories(.)

add_executable(test test.cpp pngimp/pngimp.hpp)