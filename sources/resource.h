#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <stddef.h>
#include <stdbool.h>

#define X_RESOURCES \
  X( RES_SHADER_VERTEX, "vertex.glsl" ) \
  X( RES_SHADER_FRAGMENT, "fragment.glsl" ) \
  X( RES_TEXTURE_GRAPHIC, "texture.png" ) \
  X( RES_TEXTURE_NORMALS, "normals.png" )

typedef enum {
  #define X(id, ...) id,
  X_RESOURCES
  #undef X
} res_id_t;

extern const unsigned int res_count;
extern const char* res_files[];

typedef struct {
  void* data;
  size_t len;
} resource_t;

extern bool R_Init( res_id_t* err_res_id );
extern void R_Free();
extern resource_t R_GetResource( res_id_t id );

#endif // __RESOURCE_H__


