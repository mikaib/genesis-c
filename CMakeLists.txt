cmake_minimum_required(VERSION 3.30)
project(Native C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories( SYSTEM "glad/include" )

# GL / Windows
if(WIN32)
    link_libraries(opengl32)
endif()

add_executable(Native
    glad/src/gl.c
    genesis.c
    genesis.h
    genesis_opengl.c
    genesis_opengl.h
    test.c
)
