cmake_minimum_required(VERSION 2.8)
project(PegPig)
add_executable(pig main.cpp)
set_target_properties(pig PROPERTIES COMPILE_FLAGS "-std=c++11 -stdlib=libc++ -Wall -fno-rtti -fno-exceptions -msse4")
