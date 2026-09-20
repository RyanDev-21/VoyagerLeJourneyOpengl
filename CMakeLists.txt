cmake_minimum_required(VERSION 3.20)
project(opengl_project C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_library(glad STATIC
    include/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    include/glad/include
)
target_link_libraries(glad PUBLIC
    ${CMAKE_DL_LIBS}
)

add_executable(program main.c)

target_include_directories(program PRIVATE
    include
)

target_compile_options(program PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic
    -ggdb
)

target_link_libraries(program PRIVATE
    glad
    glfw
    OpenGL::GL
    m
)
