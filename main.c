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

const char *frag_shader_source =
    "#version 330 core\n"
    "uniform vec3 objectColor;\n"
    "uniform vec3 lightColor;\n"
    "void main(){\n"
    "FragColor=vec4(objectColor*lightColor,1.0f);\n"
    "}\0";
const char *frag_shader_source_2 = "#version 330 core\n"
                                   "void main(){\n"
                                   "FragColor=vec4(1.0f);\n"
                                   "};\n";

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

float lastX = 0;
float lastY = 0;
float sensitivity = 0.01f;

camera *cam2;
void apply_direction_cam(camera *cam, float xoffset, float yoffset) {
  cam->yaw += xoffset;
  cam->pitch -= yoffset; // reverse this so that when going up will look up
  if (cam->pitch > 89.0f) {
    cam->pitch = 89.0f;
  }
  if (cam->pitch < -89.0f) {
    cam->pitch = -89.0f;
  }
  vec3 direction;
  direction[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
  direction[1] = sinf(glm_rad(cam->pitch));
  direction[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
  glm_normalize(direction);
  glm_vec3_copy(direction, cam->point_dir);
}
void cam_mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  (void)window;
  float xoffset = xpos - lastX;
  float yoffset = ypos - lastY;
  lastX = xpos;
  lastY = ypos;

  xoffset *= sensitivity;
  yoffset *= sensitivity;
  apply_direction_cam(cam2, xoffset, yoffset);
}

void cam_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  (void)window;
  (void)xoffset;
  cam2->view_angle -= (float)yoffset;
  if (cam2->view_angle < 0.1f) {
    cam2->view_angle = 0.1f;
  }
  if (cam2->view_angle > 45.0f) {
    cam2->view_angle = 45.0f;
  }
}

void cam_key_callback(camera *cam, float dt, void *user_data) {
  GLFWwindow *window = (GLFWwindow *)user_data;
  float camera_speed = 10.0f * dt;
  vec3 up = {0.0f, 1.0f, 0.0f};

  vec3 right;
  vec3 tmp;
  glm_vec3_copy(cam->point_dir, tmp);
  if (tmp[1] >= -1.0f && tmp[1] <= 1.0f) {
    tmp[1] = 0.0f;
  }
  glm_vec3_cross(tmp, up, right);
  glm_vec3_normalize(right);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cam_vec3_move(tmp, &cam->position, camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cam_vec3_move(tmp, &cam->position, -camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cam_vec3_move(right, &cam->position, camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cam_vec3_move(right, &cam->position, -camera_speed);
  }
}

void update_rotation(object *obj, float dt, void *user_data) {
  (void)user_data;
  (void)dt;
  obj->attrib.angle = sin(glfwGetTime()) * 90.0f;
}
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
  shader shader_1 = {
      .vertex = vertex_shader_source,
      .frag = frag_shader_source,
  };
  shader shader_2 = {
      .vertex = vertex_shader_source,
      .frag = frag_shader_source_2,
  };

  material *mat = create_standard_material(NULL, NORMAL_MAP, &shader_1);
  // making two different material  for only change in frag shader is worth it i
  // guess
  // need to change later
  material *mat2 = create_standard_material(NULL, NORMAL_MAP, &shader_2);
  object *obj;
  object *obj2;
  objAttrib attr = {.angle = glm_rad(40.0f),
                    .rot_vec = {1.0f, 0.0f, 0.0f},
                    .scale_vec = {1.0f, 1.0f, 1.0f}};
  glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, attr.position);
  obj = create_mesh(geo, mat, &attr);
  set_obj_update_callback(obj, update_rotation, NULL);
  glm_vec3_copy((vec3){1.0f, 1.2f, -2.0f}, attr.position);
  obj2 = create_mesh(geo, mat2, &attr);
  set_obj_update_callback(obj2, NULL, NULL);

  // this one has to really shrink down
  cam2 = create_cam((vec3){0.0f, 0.0f, 5.0f}, (vec3){0.0f, 0.0f, -1.0f}, 45.0f,
                    0.1f, 100.0f, (float)width / (float)height);
  cam2->yaw = -90.0f;
  cam_set_update_key_callback(cam2, cam_key_callback, window);
  renderer_init(window);
  renderer_set_active_cam(cam2);
  float current_time;
  float last_time;
  //
  //
  // this shouldn't be in this this should be abstracted
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cam_mouse_callback);
  glfwSetScrollCallback(window, cam_scroll_callback);
  //
  //
  // up to this
  object *obj_list[2] = {obj, obj2};
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    cam_update_ratio(cam2, (float)width / (float)height);
    current_time = glfwGetTime();
    float dt = current_time - last_time;
    last_time = current_time;
    renderer_begin_frame(dt);
    renderer_set_obj_list(obj_list, 2);
    glUseProgram(mat->shaderID);
    setVec3(mat->shaderID, "lightColor", 1.0f, 1.0f, 1.0f);
    setVec3(mat->shaderID, "objectColor", 1.0f, 0.5f, 0.31f);
    render(obj_list[0], false);
    render(obj_list[1], false);
    renderer_end_frame();
    glfwPollEvents();
  }
  destroy_geometry_data(geo);
  destroy_material_data(mat);
  destroy_material_data(mat2);
  // right now i am just doing this to work it out
  for (int i = 0; i < 2; i++) {
    destroy_obj(obj_list[i]);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
