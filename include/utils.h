#ifndef UTIL_H
#define UTIL_H
// NOTE(ME)::have to reconsider this one as the stb standard is to
// have zero dependencies
#include <glad/glad.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

GLuint compile_shader(GLenum type, const char *source);
GLuint compile_into_program(const char *vertex_shader_source,
                            const char *frag_shader_source);
void setBool(GLuint program, const char *name, bool value);
void setInt(GLuint program, const char *name, int value);
void setFloat(GLuint program, const char *name, float value);
#ifdef __cplusplus
}
#endif
#endif // UTIL_H

#ifdef UTIL_IMPLEMENTATION
#undef UTIL_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
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

GLuint compile_shader(GLenum type, const char *source) {
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

#endif // UTIL_IMPLEMENTATION
