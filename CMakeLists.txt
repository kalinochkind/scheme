cmake_minimum_required(VERSION 3.7)
project(scheme)

include_directories(include)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -pedantic")

set(SOURCE_FILES src/main.cpp src/parser.cpp src/eval.cpp src/schemeobject.cpp src/std/startup.cpp src/std/special_forms.cpp src/std/functions.cpp)
add_executable(scheme ${SOURCE_FILES})