#ifndef __LINTRANS_H__
#define __LINTRANS_H__

#define LINMATH_H_ROW_MAJOR
#define LINMATH_H_STD_BUILDERS
#include <linmath/linmath.h>

extern void ltSetScale( float scaleX, float scaleY, float scaleZ );
extern void ltSetRotate( float rotateX, float rotateY, float rotateZ );
extern void ltSetWorldPos( float posX, float posY, float posZ );
extern void ltSetCamera( vec3 pos, vec3 target, vec3 up );
extern void ltSetPerspective( float aspect, float yFOV, float near, float far );

extern void ltGetMW( mat4x4 result ); // model-world transformation
extern void ltGetWVP( mat4x4 result ); // world-view-projection transformation

#endif // __LINTRANS_H__
