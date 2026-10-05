#ifndef CAM_H
#define CAM_H
#include <lib.h>
typedef struct camera camera;
typedef void (*update_callback)(camera *cam, float dt, void *update_data);

typedef void (*update_mouse_callback)(camera *cam, void *update_data);

struct camera {
  vec3 point_dir;
  vec3 position;
  float yaw;
  float pitch;
  float view_angle;
  float ratio;
  float near;
  float far;
  void *update_key_data;
  void *update_mouse_data;
  // for manipulating stuff
  //(NOTE::FOR ME) i don't really like this one maybe change later
  update_callback update_key_func;
  update_mouse_callback update_mouse_func;
  bool hide_mouse_pos;
};

camera *create_cam(vec3 position, vec3 point_dir, float view_angle, float near,
                   float far, float ratio);
void cam_set_update_key_callback(camera *cam, update_callback, void *data);
void cam_set_update_mouse_callback(camera *cam, update_mouse_callback,
                                   void *data);

void cam_update(camera *cam, float dt);
void cam_upload(const camera *cam);
void cam_system_init(void);
void cam_system_shutdown(void);
void cam_destroy(camera *cam);

void cam_update_ratio(camera *cam, float ratio);
#endif // CAM_H
