#define UTIL_IMPLEMENTATION
#include "include/utils.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
typedef enum {
  VERTEX,
  FRAG,
} shader_type;

float vertices[] = {
    -0.25f, 0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // top left(tri left)
    0.0f,   -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right(tri left)
    -0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left(tri left)
    0.25f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // top right(tri right)
    0.5f,   -0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // bottom right(tri right)
};

unsigned int indices[] = {0, 1, 2, 1, 4, 3};
const char *vertex_shader_source = "#version 330 core\n"
                                   "layout (location=0) in vec3 aPos;\n"
                                   "layout (location=1) in vec3 inColor;\n"
                                   "out vec3 aColor;\n"
                                   "void main(){\n"
                                   "gl_Position=vec4(aPos,1.0f);\n"
                                   "aColor=inColor;\n"
                                   "}\0";

const char *frag_shader_source = "#version 330 core\n"
                                 "out vec4 color;\n"
                                 "in vec3 aColor;\n"
                                 "void main(){\n"
                                 "color=vec4(aColor,1.0f);\n"
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
  (void)mods; // for the sake of compiler wwarnings
  (void)scanCode;
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
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
  GLuint VBO;
  GLuint VAO;
  GLuint EBO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    float time_val = glfwGetTime();
    float green_val = (sin(time_val) / 2.0f + 0.5f);
    float red_val = cos(time_val);
    int vertex_color_loc = glGetUniformLocation(program, "g_color");
    glUniform4f(vertex_color_loc, red_val, green_val, 0.0f, 1.0f);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteVertexArrays(1, &VAO);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
