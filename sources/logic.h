#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <stdbool.h>

#include "lintrans.h"

#define LIGHT_INTENSITY_UNIT 50

#define X_KEYS \
  X( GO_FORWARD ) \
  X( GO_BACKWARD ) \
  X( STEP_LEFT ) \
  X( STEP_RIGHT ) \
  X( AMBIENT_MORE ) \
  X( AMBIENT_LESS ) \
  X( DIFFUSE_MORE ) \
  X( DIFFUSE_LESS )

typedef enum {
  #define X(id) id,
  X_KEYS
  #undef X
} key_id_t;

typedef struct {
  double xoff, yoff;
  bool update;
} mouse_t;

typedef struct {
  vec3_t position, target, up;
} camera_t;

typedef struct {
  vec3 color;
  int ambient;
  vec3 direction;
  int diffuse;
} light_t;

extern bool key_st[];
extern mouse_t mouse_off;

extern bool Rotating;
extern bool NormalMapping;
extern float RotateY;

extern camera_t camera;
extern light_t light;

extern bool L_Init();
extern void L_Update();

#endif // __LOGIC_H__
