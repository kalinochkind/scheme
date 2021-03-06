cmake_minimum_required(VERSION 3.7)
project(scheme)

include_directories(include)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -pedantic -Wno-c99-extensions")

set(COMMON_FILES src/parser.cpp src/eval.cpp src/schemeobject.cpp src/std/startup.cpp src/std/special_forms/definitions.cpp src/std/builtins/util.cpp src/std/registry.cpp src/std/builtins/math.cpp src/std/builtins/io.cpp src/std/builtins/typechecks.cpp src/quoting.cpp src/std/special_forms/controlflow.cpp src/std/special_forms/quoting.cpp src/std/special_forms/util.cpp src/std/special_forms/promises.cpp src/char.cpp src/std/builtins/chars.cpp src/std/builtins/strings.cpp src/std/builtins/lists.cpp src/std/builtins/vectors.cpp src/std/builtins/symbols.cpp src/std/builtins/cells.cpp src/std/builtins/promises.cpp src/std/builtins/weak.cpp src/std/builtins/pairs.cpp src/std/builtins/procedures.cpp src/std/builtins/environments.cpp)
set(SOURCE_FILES src/main.cpp ${COMMON_FILES})
set(TEST_FILES src/test/test_main.cpp ${COMMON_FILES})

add_executable(scheme ${SOURCE_FILES})
add_executable(scheme_test ${TEST_FILES})
