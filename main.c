#include "cglm/vec3.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <assert.h>
#include <camera.h>
#include <lib.h>
#include <renderer.h>
#include <stdbool.h>
#include <stdio.h>

const char *vertex_shader_source =
    "#version 330 core\n"
    "void main(){\n"
    "gl_Position=_projection*_view*u_model*vec4(position,1.0f);\n"
    "a_color=color;\n"
    "a_textCoord=textureCoord;\n"
    "}\0";

const char *frag_shader_source = "#version 330 core\n"
                                 "void main(){\n"
                                 "FragColor=texture(texture1,a_textCoord);\n"
                                 "}\0";

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error:%d , Description:%s\n", error, description);
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  (void)window; // for the sake of warning

  glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow *window, int key, int scanCode, int action,
                         int mods) {
  /* float cam_speed = 0.5f; */
  (void)mods; // for the sake of compiler wwarnings
  (void)scanCode;
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void cam_vec3_move(vec3 points, vec3 *pos, float speed) {
  vec3 move;
  glm_vec3_scale(points, speed, move);
  glm_vec3_add(move, *pos, (float *)pos);
}

void cam_callback(camera *cam, float dt, void *user_data) {
  GLFWwindow *window = (GLFWwindow *)user_data;
  float camera_speed = 10.0f * dt;
  vec3 up = {0.0f, 1.0f, 0.0f};
  vec3 right;
  glm_vec3_cross(cam->point_dir, up, right);
  glm_vec3_normalize(right);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cam_vec3_move(cam->point_dir, &cam->position, camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cam_vec3_move(cam->point_dir, &cam->position, -camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cam_vec3_move(right, &cam->position, camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cam_vec3_move(right, &cam->position, -camera_speed);
  }
}

/* void process_input(GLFWwindow *window, perspective *cam, float delta_time) {
 */
/*   float camera_speed = 10.0f * delta_time; */
/*   vec3 up = {0.0f, 1.0f, 0.0f}; */
/*   vec3 right; */
/*   glm_vec3_cross(cam->points, up, right); */
/*   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { */
/*     cam_vec3_move(cam->points, &cam->position, camera_speed); */
/*   } */
/*   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { */
/*     cam_vec3_move(cam->points, &cam->position, -camera_speed); */
/*   } */
/*   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { */
/*     cam_vec3_move(right, &cam->position, camera_speed); */
/*   } */
/*   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { */
/*     cam_vec3_move(right, &cam->position, -camera_speed); */
/*   } */
/* } */
/**/
int main(void) {
  int width = 640;
  int height = 640;

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    return -1;
  }
  // Target openGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // To removes the legacy funtions:glBegin/glEnd
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(width, height, "Program with glfw", NULL, NULL);

  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwMakeContextCurrent(window);

  int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  if (version == 0) {
    printf("Failed to init the openGL context\n");
    return -1;
  }
  geometry *geo = create_cube_geometry();
  shader shader = {
      .vertex = vertex_shader_source,
      .frag = frag_shader_source,
  };
  material *mat =
      create_standard_material("./container.jpg", NORMAL_MAP, &shader);
  vec3 cubePositions[] = {
      {0.0f, 0.0f, 0.0f},     {2.0f, 0.0f, -15.0f}, {-1.5f, -2.2f, -2.5f},
      {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f},
      {1.3f, -2.0f, -2.5f},   {1.5f, 2.0f, -2.5f},  {1.5f, 0.2f, -1.5f},
      {-1.3f, 1.0f, -1.5f},
  };
  object *obj_list[10];
  float time_val = glfwGetTime();
  for (int i = 0; i < 10; i++) {
    objAttrib attr = {.angle = glm_rad(cos(time_val) * 90.0f),
                      .rot_vec = {1.0f, 0.0f, 0.0f},
                      .scale_vec = {1.0f, 1.0f, 1.0f}};
    glm_vec3_copy(cubePositions[i], attr.position);
    obj_list[i] = create_mesh(geo, mat, &attr);
  }

  camera *cam = create_cam((vec3){0.0f, 0.0f, 5.0f}, (vec3){0.0f, 0.0f, -1.0f},
                           45.0f, 0.1f, 100.0f, (float)width / (float)height);
  cam_set_update_callback(cam, cam_callback, window);
  renderer_init(window);
  renderer_set_active_cam(cam);
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    cam_update_ratio(cam, (float)width / (float)height);
    float dt = 1.0f / 60.0f;
    renderer_begin_frame(dt);
    for (int i = 0; i < 10; i++) {
      render(obj_list[i], i % 2 == 0 ? true : false);
    }
    renderer_end_frame();
    glfwPollEvents();
  }
  destroy_geometry_data(geo);
  destroy_material_data(mat);
  // right now i am just doing to work it out
  for (int i = 0; i < 10; i++) {
    destroy_obj(obj_list[i]);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
