#define UTIL_IMPLEMENTATION
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "include/utils.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <cglm/cglm.h>
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
                                   "uniform mat4 trans;\n"
                                   "void main(){\n"
                                   "gl_Position=trans*vec4(aPos,1.0f);\n"
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
  (void)mods; // for the sake of compiler wwarnings
  (void)scanCode;
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static GLuint read_and_bind_texture(const char *path, GLuint program,
                                    const char *uniform_label, GLenum format,
                                    GLenum target) {
  GLuint texture_buff;
  glGenTextures(1, &texture_buff);
  glActiveTexture(target);
  glBindTexture(GL_TEXTURE_2D, texture_buff);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_set_flip_vertically_on_load(true);
  int img_width, img_height, nrrChannels;
  unsigned char *data_image =
      stbi_load(path, &img_width, &img_height, &nrrChannels, 0);
  if (data_image) {
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format,
                 GL_UNSIGNED_BYTE, data_image);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("[ERROR]failed to create a image data\n");
    stbi_image_free(data_image);
    glDeleteTextures(1, &texture_buff);
    return 0;
  }
  glUseProgram(program);
  setInt(program, uniform_label, (int)target - GL_TEXTURE0);
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

static bool apply_trans_matrix(GLuint program, const char *uniform_label,
                               vec3 translate_vec, vec3 rot_vec, float angle,
                               vec3 scale_vec) {

  GLint loc = glGetUniformLocation(program, uniform_label);
  assert(loc != -1 && "Failed to find the uniform loc");
  mat4 trans_mat = GLM_MAT4_IDENTITY_INIT;
  if (translate_vec) {
    glm_translate(trans_mat, translate_vec);
  }
  if (rot_vec) {
    if (angle) {
      glm_rotate(trans_mat, glm_rad(angle), rot_vec);
    } else {
      return false;
    }
  }
  if (scale_vec) {
    glm_scale(trans_mat, scale_vec);
  }
  glUniformMatrix4fv(loc, 1, GL_FALSE, (float *)trans_mat);
  return true;
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
  geometry *data =
      create_geometry(vertices, sizeof(vertices), indices, sizeof(indices));
  geometry *data2 =
      create_geometry(vertices, sizeof(vertices), indices, sizeof(indices));
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(0);
    glBindVertexArray(data->vao);
    apply_trans_matrix(program, "trans", (vec3){0.5f, -0.5f, 0.5f},
                       (vec3){0.0f, 0.0f, 0.1f}, 180.0f,
                       (vec3){0.5f, 0.5f, 0.5f});
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(program);
    glBindVertexArray(data2->vao);
    apply_trans_matrix(program, "trans", (vec3){-0.5f, 0.5f, 0.5f},
                       (vec3){0.0f, 0.0f, 0.1f}, 90.0f,
                       (vec3){0.5f, 0.5f, 0.5f});

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteTextures(1, &texture);
  glDeleteTextures(1, &texture2);
  destroy_geometry_data(data);
  destroy_geometry_data(data2);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
