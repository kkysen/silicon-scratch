cmake_minimum_required(VERSION 3.9)
project(SiliconScratch CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(cpp)
include_directories(/home/kkysen/workspace/emsdk/emscripten/1.38.21/system/include/)

# support lto (link time optimization)
SET(CMAKE_AR "gcc-ar")
SET(CMAKE_RANLIB "gcc-ranlib")


set(CMAKE_C_OPTIMIZATION_FLAGS "-O0 -flto")
set(CMAKE_C_ERROR_FLAGS "-Wall -Werror -Wextra")
set(CMAKE_C_NO_ERROR_FLAGS "-Wno-error=unused-function -Wno-error=unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_ERROR_FLAGS} ${CMAKE_C_NO_ERROR_FLAGS} ${CMAKE_C_OPTIMIZATION_FLAGS}")

set(SOURCE_FILES
        src/main/core/main.cpp
        src/main/core/main.h
        )

set(TEST_FILES
        src/test/test.cpp
        src/test/test.h
        src/test/Test.cpp
        src/test/Test.h
        src/test/Tests.cpp
        src/test/Tests.h
        )

add_library(SiliconScratch STATIC ${SOURCE_FILES})

add_executable(SiliconScratch.test ${TEST_FILES})

target_link_libraries(SiliconScratch)