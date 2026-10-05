#ifndef LIB_H
#define LIB_H
#include "cglm/types.h"
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  size_t vertex_count;
  size_t indices_count;
} geometry;
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

typedef struct {
  vec3 position;
  vec3 scale_vec;
  vec3 rot_vec;
  float angle;
} objAttrib;

typedef enum {
  VERTEX,
  FRAG,
} shaderType;

// this thing need to modify when there  are serveral object moving
// on the same behaviour
typedef struct object object;
typedef void (*obj_update_callback)(object *obj, float dt, void *user_data);
struct object {
  geometry *geo;
  material *mat;
  objAttrib attrib;
  obj_update_callback obj_update;
  void *obj_data;
};

void setBool(GLuint program, const char *name, bool value);
void setInt(GLuint program, const char *name, int value);
void setFloat(GLuint program, const char *name, float value);
void setMatrix4v(GLuint program, const char *name, mat4 matrix);
void setVec3(GLuint program, const char *name, float x, float y, float z);
geometry *create_geometry(float vertices[], size_t size_vert,
                          unsigned int indices[], size_t size_idx);
void apply_trans_matrix(vec3 translate_vec, vec3 rot_vec, float angle,
                        vec3 scale_vec, mat4 *dest);
void destroy_geometry_data(geometry *data);
material *create_standard_material(const char *tex_source, text_map type,
                                   shader *shader);
void destroy_material_data(material *mat);
void render_recursive_carpet(GLuint program, float cx, float cy, float size,
                             int depth, float time_val, size_t *triangle_count);
geometry *create_cube_geometry(void);
void render(object *obj, bool wireframe);
object *create_mesh(geometry *geo, material *mat, objAttrib *attr);
void destroy_obj(object *obj);
void set_obj_update_callback(object *obj, obj_update_callback update_callback,
                             void *user_data);
void obj_update(object *obj, float dt);
#ifdef __cplusplus
}
#endif
#endif // LIB_H
