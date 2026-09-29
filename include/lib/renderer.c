#include "renderer.h"
#include "lib.h"
#include <camera.h>
static camera *active_cam;
static GLFWwindow *window;
void renderer_set_active_cam(camera *cam) { active_cam = cam; }

void renderer_init(GLFWwindow *win) {
  glEnable(GL_DEPTH_TEST);
  cam_system_init();
  window = win;
}

void renderer_begin_frame(float dt) {

  cam_update(active_cam, dt);
  cam_upload(active_cam);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(void) { glfwSwapBuffers(window); }
