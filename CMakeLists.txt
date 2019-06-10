cmake_minimum_required(VERSION 3.13)
project(fenn C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -Wl,-undefined,dynamic_lookup")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-return-type")
endif()

include_directories(src/include src/core)

set(fenn-core
        src/core/gc.c
        src/core/parser.c
        src/core/tuple.c
        src/core/util.c
        )

add_executable(fenn
        ${fenn-core}
        src/cli/main.c
        )
