#ifndef RENDERER_H
#define RENDERER_H
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <camera.h>
#include <lib.h>

// This rendering module should take all the responsibility for drawing has to
// fix that

void renderer_set_active_cam(camera *cam);
void renderer_set_obj_list(object **obj, size_t size);
void renderer_init(GLFWwindow *window);
void renderer_begin_frame(float dt);
void renderer_end_frame(void);

#endif // RENDERER_H
