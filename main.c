#define UTIL_IMPLEMENTATION
#include "include/utils.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "include/std_image.h"
typedef enum {
  VERTEX,
  FRAG,
} shader_type;

float vertices[] = {
    // positions          // colors           // texture coords
    0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
    0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
    -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
};

int indices[] = {0, 1, 2, 3, 0, 2};
const char *vertex_shader_source = "#version 330 core\n"
                                   "layout (location=0) in vec3 aPos;\n"
                                   "layout (location=1) in vec3 inColor;\n"
                                   "layout (location=2) in vec2 aCoord;\n"
                                   "out vec3 aColor;\n"
                                   "out vec2 textCoord;\n"
                                   "void main(){\n"
                                   "gl_Position=vec4(aPos,1.0f);\n"
                                   "aColor=inColor;\n"
                                   "textCoord=aCoord;\n"
                                   "}\0";

const char *frag_shader_source =
    "#version 330 core\n"
    "out vec4 color;\n"
    "in vec3 aColor;\n"
    "in vec2 textCoord;\n"
    "uniform sampler2D text;\n"
    "void main(){\n"
    "color=texture(text,textCoord)*vec4(aColor,1.0f);\n"
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

static GLuint read_and_bind_texture(const char *path) {
  GLuint texture_buff;
  glGenTextures(1, &texture_buff);
  glBindTexture(GL_TEXTURE_2D, texture_buff);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int img_width, img_height, nrrChannels;
  unsigned char *data_image =
      stbi_load(path, &img_width, &img_height, &nrrChannels, 0);
  if (data_image) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_image);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("[ERROR]failed to create a image data\n");
    stbi_image_free(data_image);
    glDeleteTextures(1, &texture_buff);
    return 0;
  }
  stbi_image_free(data_image);
  return texture_buff;
}

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
} geometry;

static geometry *create_geometry(float vertices[], size_t size_vert,
                                 int indices[], size_t size_idx) {
  geometry *data = malloc(sizeof(geometry));
  glGenVertexArrays(1, &data->vao);
  glBindVertexArray(data->vao);
  glGenBuffers(1, &data->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
  glBufferData(GL_ARRAY_BUFFER, size_vert, vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &data->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_idx, indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  return data;
}
static void destroy_geometry_data(geometry *data) {
  glDeleteBuffers(1, &data->vbo);
  glDeleteBuffers(1, &data->ebo);
  glDeleteVertexArrays(1, &data->vao);
  free(data);
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

  GLuint texture = read_and_bind_texture("./container.jpg");
  (void)texture;
  geometry *data =
      create_geometry(vertices, sizeof(vertices), indices, sizeof(indices));
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(0);
    float time_val = glfwGetTime();
    float green_val = (sin(time_val) / 2.0f + 0.5f);
    float red_val = cos(time_val);
    int vertex_color_loc = glGetUniformLocation(program, "g_color");
    glUniform4f(vertex_color_loc, red_val, green_val, 0.0f, 1.0f);
    glBindVertexArray(data->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteTextures(1, &texture);
  destroy_geometry_data(data);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
