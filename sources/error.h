#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdio.h>
#include <stdbool.h>

#define RETURN_IF_FALSE(x) \
  if (!(x)) { return false; }

#define RETURN_FALSE_IF_EQUAL(x,y) \
  if ((x) == (y)) { return false; }

#define eprintf( fmt, ... ) \
  fprintf( stderr, fmt, __VA_ARGS__ )

#define eputs( str ) \
  fputs( str, stderr )

#endif // __ERROR_H__
