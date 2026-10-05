#include "renderer.h"
#include "lib.h"
#include <GLFW/glfw3.h>
#include <camera.h>
static camera *active_cam;
static GLFWwindow *window;
static object **obj;
static size_t obj_count;
void renderer_set_active_cam(camera *cam) { active_cam = cam; }
void renderer_set_obj_list(object **obj_list, size_t size) {
  obj = obj_list;
  obj_count = size;
}
void renderer_init(GLFWwindow *win) {
  glEnable(GL_DEPTH_TEST);
  cam_system_init();
  window = win;
}

void renderer_begin_frame(float dt) {

  cam_update(active_cam, dt);
  cam_upload(active_cam);

  for (int i = 0; i < obj_count; i++) {
    assert(obj[i] && "Failed to get the object data");
    obj_update(obj[i], dt);
  }
  // need to rethink about this
  /* obj_update(object *obj, float dt) */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(void) { glfwSwapBuffers(window); }
