#include "cglm/cam.h"
#include "cglm/vec3.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <assert.h>
#include <lib/lib.h>
#include <stdbool.h>
#include <stdio.h>

/* float vertices[] = { */
/*     // positions          // colors           // texture coords */
/*     0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right */
/*     0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right */
/*     -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left */
/*     -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left */
/* }; */
/* int indices[] = {0, 1, 2, 3, 0, 2}; */

const char *vertex_shader_source =
    "#version 330 core\n"
    "layout (location=0) in vec3 aPos;\n"
    "layout (location=1) in vec2 aCoord;\n"
    "layout (location=2) in vec3 inColor;\n"
    "out vec3 aColor;\n"
    "out vec2 textCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main(){\n"
    "gl_Position=projection*view*model*vec4(aPos,1.0f);\n"
    "aColor=inColor;\n"
    "textCoord=aCoord;\n"
    "}\0";

const char *frag_shader_source =
    "#version 330 core\n"
    "out vec4 color;\n"
    "in vec3 aColor;\n"
    "in vec2 textCoord;\n"
    "uniform sampler2D texture1;\n"
    "uniform sampler2D texture2;\n"
    "void main(){\n"
    "color=mix(texture(texture1,textCoord),texture(texture2,textCoord),0.2f);\n"
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

void process_input(GLFWwindow *window, perspective *cam, float delta_time) {
  float camera_speed = 10.0f * delta_time;
  vec3 up = {0.0f, 1.0f, 0.0f};
  vec3 right;
  glm_vec3_cross(cam->points, up, right);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cam_vec3_move(cam->points, &cam->position, camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cam_vec3_move(cam->points, &cam->position, -camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cam_vec3_move(right, &cam->position, camera_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cam_vec3_move(right, &cam->position, -camera_speed);
  }
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

  GLuint program =
      compile_into_program(vertex_shader_source, frag_shader_source);

  if (program == 0) {
    return -1;
  }

  GLuint texture = read_and_bind_texture("./container.jpg", program, "texture1",
                                         GL_RGB, GL_TEXTURE0);
  GLuint texture2 = read_and_bind_texture("./awesomeface.png", program,
                                          "texture2", GL_RGBA, GL_TEXTURE1);
  geometry *data = create_cube_geometry();

  vec3 cubePositions[] = {
      {0.0f, 0.0f, 0.0f},     {2.0f, 0.0f, -15.0f}, {-1.5f, -2.2f, -2.5f},
      {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f},
      {1.3f, -2.0f, -2.5f},   {1.5f, 2.0f, -2.5f},  {1.5f, 0.2f, -1.5f},
      {-1.3f, 1.0f, -1.5f},
  };
  perspective data_cam = {
      .points = {0.0f, 0.0f, -1.0f},
      .position = {0.0f, 0.0f, 3.0f},
      .ratio = (float)width / (float)height,
      .near = 0.1f,
      .far = 1000.0f,
  };
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    float delta_time = 1.0f / 60.0f;
    process_input(window, &data_cam, delta_time);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    glBindVertexArray(0);
    glBindVertexArray(data->vao);
    vec3 center;
    glm_vec3_add(data_cam.position, data_cam.points, center);
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    for (int i = 0; i < 10; i++) {
      glm_lookat(data_cam.position, center, (vec3){0.0f, 1.0f, 0.0f}, view);
      setMatrix4v(program, "view", view);
      apply_trans_matrix(program, "projection", NULL, NULL, 45.0f, &data_cam,
                         NULL);
      apply_trans_matrix(program, "model", cubePositions[i],
                         (vec3){1.0f, 0.3f, 0.5f}, glm_rad(50.0f), NULL, NULL);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteTextures(1, &texture);
  glDeleteTextures(1, &texture2);
  destroy_geometry_data(data);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
