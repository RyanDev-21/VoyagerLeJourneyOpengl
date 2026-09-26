#ifndef LIB_H
#define LIB_H
#include "cglm/types.h"
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

// typedef struct {
//   vec3 pivot;
//   float angle;
// } camera_angle;
//
// typedef struct {
//   vec3 position;
//   camera_angle;
//   float view_angle;
//   float near;
//   float far;
//   float ratio;
// } perspective_cam;
typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
} geometry;
//
typedef enum {
  NORMAL_MAP,
  BUMP_MAP,
  OCCLUSION_MAP,
} text_map;

typedef struct {
  const char *vertex;
  const char *frag;
} shader;
typedef struct {
  GLuint textureID;
  GLuint shaderID;
} material;
//
// typedef struct {
//   geometry *geo;
//   material *mat;
// } object;
//
// typedef struct {
//   object objs[];
//
// } scene;

typedef struct {
  vec3 points;
  vec3 position;
  float ratio;
  float near;
  float far;
} perspective;

typedef struct {
  geometry *geo;
  material *mat;
  vec3 position;
} object;

GLuint compile_into_program(const char *vertex_shader_source,
                            const char *frag_shader_source);
void setBool(GLuint program, const char *name, bool value);
void setInt(GLuint program, const char *name, int value);
void setFloat(GLuint program, const char *name, float value);
GLuint read_and_bind_texture(const char *path, GLuint program,
                             const char *uniform_label, GLenum format,
                             GLenum target);
void setMatrix4v(GLuint program, const char *name, mat4 matrix);
geometry *create_geometry(float vertices[], size_t size_vert, size_t stride,
                          unsigned int indices[], size_t size_idx);
// have to refactor this to just take enum and apply it based on that
void apply_trans_matrix(GLuint program, const char *uniform_label,
                        vec3 translate_vec, vec3 rot_vec, float angle,
                        perspective *proj, vec3 scale_vec);
void destroy_geometry_data(geometry *data);
void render_recursive_carpet(GLuint program, float cx, float cy, float size,
                             int depth, float time_val, size_t *triangle_count);
geometry *create_cube_geometry();
void render(object *obj);
#ifdef __cplusplus
}
#endif
#endif // LIB_H
