#include <stdlib.h>
#include <stdio.h>

#include "workflow.h"
#include "resource.h"
#include "render.h"
#include "logic.h"

#define TERMINATE_IF_FALSE(x) \
  if (!x) { return EXIT_FAILURE; }

static void terminate() {
  main_free();
  G_Free();
  R_Free();
}

int main() {
  atexit( &terminate );

  TERMINATE_IF_FALSE( R_Init(NULL) );
  TERMINATE_IF_FALSE( main_init() );
  TERMINATE_IF_FALSE( G_Init() );
  TERMINATE_IF_FALSE( L_Init() );

  TERMINATE_IF_FALSE( main_loop() );

  puts( "done with success." );
  return EXIT_SUCCESS;
}


