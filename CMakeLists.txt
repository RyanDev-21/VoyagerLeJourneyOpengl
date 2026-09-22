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

# add_library(glm STATIC
#     include/glm/glm.c
# )
target_include_directories(glad PUBLIC
    include/glad/include
)
target_link_libraries(glad PUBLIC
    ${CMAKE_DL_LIBS}
)

add_executable(program main.c)
include(FetchContent)
FetchContent_Declare(
    cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG        v0.9.4 # Or latest release tag
)
FetchContent_MakeAvailable(cglm)
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
    cglm 
    glfw
    OpenGL::GL
    m
)
