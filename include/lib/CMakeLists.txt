add_library(gl_util STATIC lib.c camera.c renderer.c)
 
target_include_directories(gl_util PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gl_util PUBLIC
    glad
    cglm
    OpenGL::GL
    m
)
