#include "lib.h"
#include "cglm/cam.h"
#define STR_VIEW_IMPLEMENTATION
#include "string_view.h"
#include <assert.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../std_image.h"

/* void add_camera(perspective_cam *cam, scene *scene) { */
/*   GLint loc =
 * glGetUniformLocation(scene->objs->mat->custom_shader->program,); */
/* } */

static bool check_status(GLuint object, GLenum status_type, const char *label);
static void inject_and_compile(GLuint programID, const char *raw_shader) {
  const char *injection = "layout(std140) uniform;\n"
                          "uniform cam_uniform{\n"
                          "mat4 projection;\n"
                          "mat4 view ;\n"
                          "}\n;";
  StringView str = sv_from_cstr(raw_shader);
  StringView version_line = {0};
  StringView rest = sv_trim_left(str);
  StringView version_tag = sv_from_cstr("#version");
  if (rest.count >= version_tag.count &&
      memcmp(rest.data, version_tag.data, version_tag.count) == 0) {
    version_line = sv_cut_by_delim(&rest, '\n');
  }
  const char *sources[4];
  GLint lengths[4];
  int count = 0;
  if (version_line.count > 0) {
    sources[count] = version_line.data;
    lengths[count] = version_line.count;
    count++;

    sources[count] = "\n";
    lengths[count] = 1;
    count++;
  }

  sources[count] = injection;
  lengths[count] = strlen(injection);
  count++;
  sources[count] = rest.data;
  lengths[count] = rest.count;

  GLuint shaderID;
  glCreateShader(shaderID);
  glShaderSource(shaderID, 3, sources, lengths);
  glCompileShader(shaderID);
  assert(check_status(shaderID, GL_COMPILE_STATUS, "shader") &&
         "[ERROR]Failed to compiled the injected shader\n");

  glAttachShader(programID, shaderID);
  glDeleteShader(shaderID);
}

// NOTE(FOR ME):: still need to do the tex_source loading and attaching stuff
// maybe better to do with the type passed into inject_and_compile so that for
// the text_source we can only inject to the frag_source
material *createStandardMetrial(const char *tex_source, text_map type,
                                shader *shader) {
  material *mat = malloc(sizeof(material));
  assert(mat != NULL && "[ERROR]Failed to allocate the material");
  if (shader != NULL) {
    GLuint programID = glCreateProgram();
    // compile and attach separately
    if (shader->vertex != NULL) {
      inject_and_compile(programID, shader->vertex);
    }
    if (shader->frag != NULL) {
      inject_and_compile(programID, shader->frag);
    }
    // if both exits then has to link
    shader->vertex != NULL && shader->frag != NULL ? glLinkProgram(programID)
                                                   : NULL;
    assert(!check_status(programID, GL_LINK_STATUS, "program"));
  }
  return mat;
}

/* object *create_object(float pos[3], geometry *geometry, material *mat) { */
/*   object *obj = malloc(sizeof(object)); */
/*   obj->geo = geometry; */
/*   const char *texture = mat->tex_source; */
/*   obj->mat = mat; */
/*   glm_vec3_copy(obj->position, pos); */
/*   return obj; */
/* } */

void render(object *obj) {}

void setMatrix4v(GLuint program, const char *name, mat4 matrix) {
  GLuint loc = glGetUniformLocation(program, name);
  assert(loc != 0 && "Failed to get the uniform loc from the shader program");
  glUniformMatrix4fv(loc, 1, GL_FALSE, (float *)matrix);
}

geometry *create_cube_geometry() {
  float vertices[] = {
      // Front face (Z = 0.5f)
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // 0: bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // 1: bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // 2: top-right
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // 3: top-left

      // Back face (Z = -0.5f)
      0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // 4
      -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, // 5
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // 6
      0.5f, 0.5f, -0.5f, 0.0f, 1.0f,   // 7

      // Left face (X = -0.5f)
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 8
      -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // 9
      -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // 10
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // 11

      // Right face (X = 0.5f)
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // 12
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f, // 13
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // 14
      0.5f, 0.5f, 0.5f, 0.0f, 1.0f,   // 15

      // Top face (Y = 0.5f)
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,  // 16
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // 17
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // 18
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // 19

      // Bottom face (Y = -0.5f)
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 20
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // 21
      0.5f, -0.5f, 0.5f, 1.0f, 1.0f,   // 22
      -0.5f, -0.5f, 0.5f, 0.0f, 1.0f   // 23
  };

  // 36 indices: 6 indices per face (two triangles: 0-1-2 and 2-3-0 pattern)
  unsigned int indices[] = {
      0,  1,  2,  2,  3,  0,  // Front
      4,  5,  6,  6,  7,  4,  // Back
      8,  9,  10, 10, 11, 8,  // Left
      12, 13, 14, 14, 15, 12, // Right
      16, 17, 18, 18, 19, 16, // Top
      20, 21, 22, 22, 23, 20  // Bottom
  };
  geometry *data = create_geometry(vertices, sizeof(vertices),
                                   5 * sizeof(float), indices, sizeof(indices));

  return data;
}

geometry *create_geometry(float vertices[], size_t size_vert, size_t stride,
                          unsigned int indices[], size_t size_idx) {
  geometry *data = malloc(sizeof(geometry));
  glGenVertexArrays(1, &data->vao);
  glBindVertexArray(data->vao);
  glGenBuffers(1, &data->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
  glBufferData(GL_ARRAY_BUFFER, size_vert, vertices, GL_STATIC_DRAW);
  if (indices && size_idx > 0) {
    glGenBuffers(1, &data->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_idx, indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);
  } else {
    data->ebo = 0;
  }
  if (stride == 5 * sizeof(float)) {
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  } else if (stride == 8 * sizeof(float)) {
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
  }

  return data;
}
void apply_trans_matrix(GLuint program, const char *uniform_label,
                        vec3 translate_vec, vec3 rot_vec, float angle_rad,
                        perspective *proj, vec3 scale_vec) {

  GLint loc = glGetUniformLocation(program, uniform_label);
  assert(loc != -1 && "[ERROR]Failed to find the uniform loc");
  mat4 trans_mat = GLM_MAT4_IDENTITY_INIT;
  if (translate_vec) {
    glm_translate(trans_mat, translate_vec);
  }
  if (rot_vec) {
    if (angle_rad) {
      glm_rotate(trans_mat, angle_rad, rot_vec);
    } else {
      assert(angle_rad &&
             "[ERROR]Angle must be specifided when using rotation vector");
    }
  }
  if (proj) {
    if (angle_rad) {
      glm_perspective(glm_rad(angle_rad), proj->ratio, proj->near, proj->far,
                      trans_mat);
    } else {
      assert(angle_rad &&
             "[ERROR]angle_rad must be specifided when using rotation vector");
    }
  }
  if (scale_vec) {
    glm_scale(trans_mat, scale_vec);
  }
  glUniformMatrix4fv(loc, 1, GL_FALSE, (float *)trans_mat);
}

void destroy_geometry_data(geometry *data) {
  glDeleteBuffers(1, &data->vbo);
  glDeleteBuffers(1, &data->ebo);
  glDeleteVertexArrays(1, &data->vao);
  free(data);
}

GLuint read_and_bind_texture(const char *path, GLuint program,
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

void setBool(GLuint program, const char *name, bool value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}

void setInt(GLuint program, const char *name, int value) {
  glUniform1i(glGetUniformLocation(program, name), value);
}

void setFloat(GLuint program, const char *name, float value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}

static bool check_status(GLuint object, GLenum status_type, const char *label) {
  int success;
  char info_log[512];
  if (status_type == GL_COMPILE_STATUS) {
    glGetShaderiv(object, status_type, &success);
    if (!success) {
      glGetShaderInfoLog(object, 512, NULL, info_log);
      printf("[ERROR]Failed to compile %s shader :%s\n", label, info_log);
      return false;
    }
  } else {
    glGetProgramiv(object, status_type, &success);
    if (!success) {
      glGetProgramInfoLog(object, 512, NULL, info_log);
      printf("[ERROR]Failed to link the program :%s\n", info_log);
      return false;
    }
  }
  return true;
}

static GLuint compile_shader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  const char *label = (type == GL_VERTEX_SHADER ? "vertex" : "frag");
  if (!check_status(shader, GL_COMPILE_STATUS, label)) {
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

GLuint compile_into_program(const char *vertex_shader_source,
                            const char *frag_shader_source) {

  GLuint vertex_shader = 0;
  GLuint frag_shader = 0;
  GLuint program = 0;

  vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
  if (vertex_shader == 0)
    return 0;
  frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_shader_source);
  if (frag_shader == 0) {
    glDeleteShader(vertex_shader);
    return 0;
  }
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, frag_shader);
  glLinkProgram(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(frag_shader);

  if (!check_status(program, GL_LINK_STATUS, "program")) {
    glDeleteProgram(program);
    program = 0;
  };
  return program;
}
