#include "lib.h"
#include "cglm/mat4.h"
#define STR_VIEW_IMPLEMENTATION
#include "string_view.h"
#include <assert.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../std_image.h"

#define CAM_BINDING 0

static GLuint compile_into_program(const char *vertex_shader_source,
                                   const char *frag_shader_source);

static GLuint read_and_bind_texture(const char *path, GLuint program,
                                    const char *uniform_label, GLenum format,
                                    GLenum target);
static bool check_status(GLuint object, GLenum status_type, const char *label);

static GLuint u_model_loc;

//(NOTE::For me)Has to rethink about this
static void set_obj_attrib(object *obj, objAttrib *attr) {
  mat4 trans_model = GLM_MAT4_IDENTITY_INIT;
  apply_trans_matrix(attr->position, attr->rot_vec, glm_rad(attr->angle),
                     attr->scale_vec, &trans_model);
  if (!u_model_loc) {
    u_model_loc = glGetUniformLocation(obj->mat->shaderID, "u_model");
  }
  glUniformMatrix4fv(u_model_loc, 1, GL_FALSE, (float *)trans_model);
}
// the mdoel matrix has to transform per frame so
void render(object *obj, bool wireframe) {
  glUseProgram(obj->mat->shaderID);
  set_obj_attrib(obj, &obj->attrib);
  glBindVertexArray(obj->geo->vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, obj->mat->textureID);
  wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
            : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  obj->geo->ebo > 0 ? glDrawElements(GL_TRIANGLES, obj->geo->indices_count,
                                     GL_UNSIGNED_INT, 0)
                    : glDrawArrays(GL_TRIANGLES, 0, obj->geo->vertex_count);
}

//(NOTE ::FOR ME) this funciton is implemented so dumb right now i mean when the
// user have
// other reference to it then deleting like this very bad
void destroy_obj(object *obj) {
  /* destroy_geometry_data(obj->geo); */
  /* destroy_material_data(obj->mat); */
  free(obj);
}

object *create_mesh(geometry *geo, material *mat, objAttrib *attr) {
  object *obj = malloc(sizeof(object));
  assert(obj && "Failed to allocate the space for object");
  obj->geo = geo;
  obj->mat = mat;
  obj->attrib = *attr;
  set_obj_attrib(obj, attr);
  return obj;
}

static const char *injection_vertex =
    "layout(std140) uniform;\n"
    "uniform cam_uniform{\n"
    "mat4 _projection;\n"
    "mat4 _view ;\n"
    "}\n;"
    "uniform mat4 u_model\n;"
    "layout (location=0)in vec3 position;\n"
    "layout (location=1) in vec3 color;\n"
    "layout (location=2)in vec2 textureCoord;\n"
    "out vec3 a_color;\n"
    "out vec2 a_textCoord;\n";

static const char *injection_frag = "out vec4 FragColor;\n"
                                    "in vec3 a_color;\n"
                                    "in vec2 a_textCoord;\n"
                                    "uniform sampler2D texture1;\n";
static void inject_and_compile(GLuint programID, const char *raw_shader,
                               shaderType type) {
  const char *header = type == VERTEX ? injection_vertex : injection_frag;
  GLenum gl_type = type == VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
  StringView rest = sv_trim_left(sv_from_cstr(raw_shader));
  StringView version_tag = sv_from_cstr("#version");
  assert(rest.count >= version_tag.count &&
         memcmp(rest.data, version_tag.data, version_tag.count) == 0 &&
         "Cannot find the version core init for glsl");
  // parse the version core line
  StringView version_line = sv_cut_by_delim(&rest, '\n');
  printf(STR_FMT, STR_ARG(version_line));
  const char *sources[4] = {version_line.data, "\n", header, rest.data};
  GLint lengths[4] = {version_line.count, 1, (GLuint)strlen(header),
                      rest.count};
  GLuint shaderID = glCreateShader(gl_type);
  glShaderSource(shaderID, 4, sources, lengths);
  glCompileShader(shaderID);
  assert(check_status(shaderID, GL_COMPILE_STATUS, "shader") &&
         "[ERROR]Failed to compiled the injected shader\n");

  glAttachShader(programID, shaderID);
  glDeleteShader(shaderID);
}

material *create_standard_material(const char *tex_source, text_map type,
                                   shader *shader) {
  material *mat = calloc(1, sizeof(material));
  assert(mat != NULL && "[ERROR]Failed to allocate the material");
  if (shader != NULL) {
    GLuint programID = glCreateProgram();
    // compile both
    if (shader->vertex != NULL && shader->frag != NULL) {
      inject_and_compile(programID, shader->vertex, VERTEX);
      inject_and_compile(programID, shader->frag, FRAG);
    }
    assert(shader->vertex != NULL && shader->frag != NULL &&
           "Failed to compile the shaders\n");
    // if both exits then has to link
    glLinkProgram(programID);
    assert(check_status(programID, GL_LINK_STATUS, "program") &&
           "[ERROR]Failed to link both program\n");
    mat->shaderID = programID;
    GLuint idx = glGetUniformBlockIndex(mat->shaderID, "cam_uniform");
    if (idx != GL_INVALID_INDEX) {
      glUniformBlockBinding(mat->shaderID, idx, CAM_BINDING);
    }
  }

  // if text_source exits then we need to load and bind them
  if (tex_source != NULL) {
    GLuint textureID = read_and_bind_texture(tex_source, mat->shaderID,
                                             "texture1", GL_RGB, GL_TEXTURE0);
    mat->textureID = textureID;
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

void setMatrix4v(GLuint program, const char *name, mat4 matrix) {
  glUseProgram(program);
  GLuint loc = glGetUniformLocation(program, name);
  assert(loc != 0 && "Failed to get the uniform loc from the shader program");
  glUniformMatrix4fv(loc, 1, GL_FALSE, (float *)matrix);
}

geometry *create_cube_geometry() {
  float vertices[] = {
      // Front face (Z = 0.5f)
      -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // 0: bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // 1: bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,   // 2: top-right
      -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // 3: top-left

      // Back face (Z = -0.5f)
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  // 4
      -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // 5
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  // 6
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,   // 7

      // Left face (X = -0.5f)
      -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // 8
      -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // 9
      -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,   // 10
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // 11

      // Right face (X = 0.5f)
      0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  // 12
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // 13
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  // 14
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,   // 15

      // Top face (Y = 0.5f)
      -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  // 16
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,   // 17
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  // 18
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 19

      // Bottom face (Y = -0.5f)
      -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // 20
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // 21
      0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,   // 22
      -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f   // 23
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
  geometry *data =
      create_geometry(vertices, sizeof(vertices), indices, sizeof(indices));

  return data;
}

geometry *create_geometry(float vertices[], size_t size_vert,
                          unsigned int indices[], size_t size_idx) {
  geometry *data = calloc(1, sizeof(geometry));
  glGenVertexArrays(1, &data->vao);
  glBindVertexArray(data->vao);
  glGenBuffers(1, &data->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
  glBufferData(GL_ARRAY_BUFFER, size_vert, vertices, GL_STATIC_DRAW);
  size_t stride = 8 * sizeof(float);
  data->vertex_count = size_vert / stride;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  if (indices && size_idx > 0) {
    glGenBuffers(1, &data->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_idx, indices, GL_STATIC_DRAW);
    data->indices_count = size_idx / sizeof(unsigned int);
  } else {
    data->ebo = 0;
  }

  return data;
}
void apply_trans_matrix(vec3 translate_vec, vec3 rot_vec, float angle_rad,
                        vec3 scale_vec, mat4 *dest) {
  mat4 trans_mat = GLM_MAT4_IDENTITY_INIT;
  if (translate_vec) {
    glm_translate(trans_mat, translate_vec);
  }
  if (rot_vec) {
    if (angle_rad) {
      glm_rotate(trans_mat, angle_rad, rot_vec);
    } else {
      assert(angle_rad == 0.0f &&
             "[ERROR]Angle must be specifided when using rotation vector");
    }
  }
  /* if (proj) { */
  /*   if (angle_rad) { */
  /*     glm_perspective(glm_rad(angle_rad), proj->ratio, proj->near, proj->far,
   */
  /*                     trans_mat); */
  /*   } else { */
  /*     assert(angle_rad && */
  /*            "[ERROR]angle_rad must be specifided when using rotation
   * vector"); */
  /*   } */
  /* } */
  if (scale_vec) {
    glm_scale(trans_mat, scale_vec);
  }
  glm_mat4_copy(trans_mat, *dest);
}

void destroy_material_data(material *mat) {
  glDeleteProgram(mat->shaderID);
  glDeleteTextures(1, &mat->textureID);
  free(mat);
}

void destroy_geometry_data(geometry *data) {
  GLuint buffers[2] = {data->vbo, data->ebo};
  glDeleteBuffers(2, buffers);
  glDeleteVertexArrays(1, &data->vao);
  free(data);
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

static GLuint compile_into_program(const char *vertex_shader_source,
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
void setBool(GLuint program, const char *name, bool value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}

void setInt(GLuint program, const char *name, int value) {
  glUniform1i(glGetUniformLocation(program, name), value);
}

void setFloat(GLuint program, const char *name, float value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}
