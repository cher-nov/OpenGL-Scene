#include "logic.h"

#include "routines.h"

#define STEP_SIZE 0.07f
#define TURN_ANGLE 0.05f
#define MOUSE_SENS 0.2f

#define X(...) +1
bool key_st[X_KEYS];
#undef X

mouse_t mouse_off = {0.0, 0.0, false};

bool Rotating = false;
bool NormalMapping = true;
float RotateY = 0.0f;

camera_t camera =
  { iVEC3T(0.0f,0.0f,-5.0f), iVEC3T(0.0f,0.0f,1.0f), iVEC3T(0.0f,1.0f,0.0f) };
light_t light = {
  {0.5f, 0.7f, 1.0f}, LIGHT_INTENSITY_UNIT,
  {0.0f, 0.0f, 0.0f}, LIGHT_INTENSITY_UNIT
};

static void clear_keys() {
  #define X(id) \
    key_st[id] = false;
  X_KEYS
  #undef X
}

bool L_Init() {
  clear_keys();
  return true;
}

void L_Update() {
  if (Rotating)
    RotateY = fmod( RotateY+0.8f, 360.0f );

  /* MOVEMENT CONTROL
  * =============================================================== */

  vec3 cam_step;
  vec3_scale( cam_step, camera.target.v, STEP_SIZE );

  if (key_st[GO_FORWARD]) {
    vec3_add( camera.position.v, camera.position.v, cam_step );
  }
  if (key_st[GO_BACKWARD]) {
    vec3_sub( camera.position.v, camera.position.v, cam_step );
  }

  vec3 side;
  vec3_mul_cross( side, camera.target.v, camera.up.v );
  vec3_norm( side, side );
  vec3_scale( side, side, STEP_SIZE );

  if (key_st[STEP_LEFT]) {
    vec3_sub( camera.position.v, camera.position.v, side );
  }
  if (key_st[STEP_RIGHT]) {
    vec3_add( camera.position.v, camera.position.v, side );
  }

  if (mouse_off.update) {
    static float yaw = 90.0f; //direction
    static float pitch = 0.0f; //tilt

    mouse_off.xoff *= MOUSE_SENS;
    mouse_off.yoff *= MOUSE_SENS;

    yaw = fmod( yaw + mouse_off.xoff, 360.0f );
    pitch += mouse_off.yoff;
    if (pitch >  89.0f) { pitch =  89.0f; }
    if (pitch < -89.0f) { pitch = -89.0f; }

    camera.target.i.x = cosf( deg2rad(yaw) ) * cosf( deg2rad(pitch) );
    camera.target.i.y = sinf( deg2rad(pitch) );
    camera.target.i.z = sinf( deg2rad(yaw) ) * cosf( deg2rad(pitch) );
    vec3_norm( camera.target.v, camera.target.v );

    mouse_off.update = false;
  }

  /* LIGHT CONTROL
  * =============================================================== */

  //camera flashlight shit, don't use
/*
  vec3_dup( light.direction, camera.target.v );
  //vec3_norm( light.direction, light.direction );
  vec3_sub( light.direction, light.direction, camera.position.v );
  //vec3_sub( light.direction, light.direction, camera.up.v );
  //vec3_norm( light.direction, light.direction );
*/
  vec3_dup( light.direction, VEC3(1.0f, 0.0f, 0.0f) );

  if (key_st[AMBIENT_LESS]) {
    if (light.ambient > 0) light.ambient--;
    key_st[AMBIENT_LESS] = false;
  }
  if (key_st[AMBIENT_MORE]) {
    if (light.ambient < LIGHT_INTENSITY_UNIT*3) light.ambient++;
    key_st[AMBIENT_MORE] = false;
  }

  if (key_st[DIFFUSE_LESS]) {
    if (light.diffuse > 0) light.diffuse--;
    key_st[DIFFUSE_LESS] = false;
  }
  if (key_st[DIFFUSE_MORE]) {
    if (light.diffuse < LIGHT_INTENSITY_UNIT*3) light.diffuse++;
    key_st[DIFFUSE_MORE] = false;
  }

  //clear_keys();
}


