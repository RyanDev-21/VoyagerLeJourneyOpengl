#include "lib.h"
#include <camera.h>

static GLuint cam_ubo;

void cam_system_init(void) {
  glGenBuffers(1, &cam_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, cam_ubo);
  glBufferData(GL_UNIFORM_BUFFER, 2 * 16 * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, cam_ubo);
}
camera *create_cam(vec3 position, vec3 point_dir, float view_angle, float near,
                   float far, float ratio) {
  camera *cam = calloc(1, sizeof(camera));
  assert(cam && "Failed to allocated the space of the camera");
  glm_vec3_copy(position, cam->position);
  glm_vec3_copy(point_dir, cam->point_dir);
  cam->view_angle = view_angle;
  cam->near = near;
  cam->far = far;
  cam->ratio = ratio;
  cam->yaw = -90.0f;
  return cam;
};
void cam_system_shutdown(void) { glDeleteBuffers(1, &cam_ubo); }
void cam_destroy(camera *cam) { free(cam); }

// this is per frame callback
void cam_set_update_key_callback(camera *cam, update_callback update_func,

                                 void *data) {
  cam->update_key_func = update_func;
  cam->update_key_data = data;
}

void cam_set_update_mouse_callback(camera *cam, mouse_callback update_func,
                                   void *data) {
  cam->update_mouse_func = update_func;
  cam->update_mouse_data = data;
}
void cam_set_update_scroll_callback(camera *cam, mouse_callback update_func,
                                    void *data) {
  cam->update_scroll_func = update_func;
  cam->update_scroll_data = data;
}

void cam_update_ratio(camera *cam, float ratio) { cam->ratio = ratio; }

void cam_upload(const camera *cam) {
  mat4 projection, view;
  glm_look((float *)cam->position, (float *)cam->point_dir,
           (vec3){0.0f, 1.0f, 0.0f}, view);
  glm_perspective(glm_rad(cam->view_angle), cam->ratio, cam->near, cam->far,
                  projection);
  glBindBuffer(GL_UNIFORM_BUFFER, cam_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), projection);
  glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(mat4), view);
}

void cam_update(camera *cam, float dt) {
  if (cam->update_key_func) {
    cam->update_key_func(cam, dt, cam->update_key_data);
  }
}
