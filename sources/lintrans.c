#include "lintrans.h"

#include <stdlib.h>
#include <stdarg.h>
#include "routines.h"

static vec3_t atScale = iVEC3T(1.0f, 1.0f, 1.0f);
static vec3_t atWorldPos = iVEC3T(0.0f, 0.0f, 0.0f);
static vec3_t atRotate = iVEC3T(0.0f, 0.0f, 0.0f);

struct {
  float aspect, yFOV, near, far;
} atProjection;

//note: target is relative to pos
struct {
  vec3 pos, target, up;
} atCamera;

/*
static void print_mx(mat4x4 M) {
  for(int i=0; i<4; ++i) {
    for(int j=0; j<4; ++j) {
      printf("%.2f\t", M[j][i]);
    }
    printf("\n");
  }
  printf("\n");
  system("pause>nul");
}
*/

//warning: don't use with n < 2
typedef vec4* mat4x4_ptr; //vec4 instead of mat4x4 supresses warnings
static void mat4x4_mul_seq( mat4x4 R, int n, ... ) {
  va_list args;
  va_start( args, n );
  for( int i = 0; i < n-1; ++i ) {
    if (i == 0) { mat4x4_dup( R, va_arg( args, mat4x4_ptr ) ); }
    mat4x4_mul( R, va_arg( args, mat4x4_ptr ), R );
  }
  va_end( args );
}

void ltSetScale( float scaleX, float scaleY, float scaleZ ) {
  atScale.i.x = scaleX;
  atScale.i.y = scaleY;
  atScale.i.z = scaleZ;
}

void ltSetRotate( float rotateX, float rotateY, float rotateZ ) {
  atRotate.i.x = deg2rad(rotateX);
  atRotate.i.y = deg2rad(rotateY);
  atRotate.i.z = deg2rad(rotateZ);
}

void ltSetWorldPos( float posX, float posY, float posZ ) {
  atWorldPos.i.x = posX;
  atWorldPos.i.y = posY;
  atWorldPos.i.z = posZ;
}

void ltSetCamera( vec3 pos, vec3 target, vec3 up ) {
  vec3_dup( atCamera.pos, pos );
  vec3_add( atCamera.target, pos, target );
  vec3_dup( atCamera.up, up );
}

void ltSetPerspective( float aspect, float yFOV, float near, float far ) {
  atProjection.aspect = aspect;
  atProjection.yFOV = deg2rad(yFOV);
  atProjection.near = near;
  atProjection.far = far;
}

void ltGetMW( mat4x4 result ) {
  mat4x4 mxScale;
  mat4x4_identity( mxScale );
  mat4x4_scale_aniso( mxScale, mxScale, atScale.i.x, atScale.i.y, atScale.i.z );

  mat4x4 mxRotate;
  mat4x4_identity( mxRotate );
  mat4x4_rotate_Z( mxRotate, mxRotate, atRotate.i.z );
  mat4x4_rotate_Y( mxRotate, mxRotate, atRotate.i.y );
  mat4x4_rotate_X( mxRotate, mxRotate, atRotate.i.x );

  mat4x4 mxTranslate;
  mat4x4_translate( mxTranslate,
    atWorldPos.i.x, atWorldPos.i.y, atWorldPos.i.z );

  mat4x4_mul_seq( result, 3, mxScale, mxRotate, mxTranslate );
}

void ltGetWVP( mat4x4 result ) {
  mat4x4 mxCamera; //aka "look-at matrix"
  mat4x4_look_at( mxCamera, atCamera.pos, atCamera.target, atCamera.up );

  mat4x4 mxProjection;
  mat4x4_perspective( mxProjection, atProjection.yFOV, atProjection.aspect,
    atProjection.near, atProjection.far );

  mat4x4_mul_seq( result, 2, mxCamera, mxProjection );
}


