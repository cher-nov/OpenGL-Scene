#ifndef __ROUTINES_H__
#define __ROUTINES_H__

#include <math.h>
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

#define NUMOFE(array) (sizeof(array)/sizeof(array[0]))

#define deg2rad(x) ((x) / 180.0f * M_PI)
#define rad2deg(x) ((x) * 180.0f / M_PI)

#endif // __ROUTINES_H__
