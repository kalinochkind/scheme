cmake_minimum_required(VERSION 3.7)
project(scheme)

include_directories(include)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -pedantic -Wno-c99-extensions")

set(SOURCE_FILES src/main.cpp src/parser.cpp src/eval.cpp src/schemeobject.cpp src/std/startup.cpp src/std/special_forms/definitions.cpp src/std/builtins/util.cpp src/std/registry.cpp src/std/builtins/math.cpp src/std/builtins/io.cpp src/std/builtins/typechecks.cpp src/quoting.cpp src/std/special_forms/controlflow.cpp src/std/special_forms/quoting.cpp src/std/special_forms/util.cpp src/std/special_forms/promises.cpp include/char.h src/char.cpp src/std/builtins/chars.cpp)
add_executable(scheme ${SOURCE_FILES})
