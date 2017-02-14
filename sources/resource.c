#include "resource.h"

#include <stdlib.h>
#include <memory.h>

#include "error.h"

#define NULL_RES {NULL, 0}
#define NO_RESOURCE (resource_t)NULL_RES

#define X(...) +1
static resource_t res_info[X_RESOURCES] = {NULL_RES};
const unsigned int res_count = X_RESOURCES;
const char* res_files[X_RESOURCES] = {
  #undef X
  #define X(id,name) name,
  X_RESOURCES
};
#undef X

static void* load_file( const char* fname, size_t* fsize ) {
  FILE* rfile = fopen(fname, "rb");
  if (rfile == NULL) { return NULL; }

  fseek( rfile, 0, SEEK_END );
  int rsize = ftell( rfile );
  fseek( rfile, 0, SEEK_SET );

  char* rdata = malloc( rsize+1 );
  if (rdata != NULL) {
    fread( rdata, sizeof(char), rsize, rfile );
    rdata[rsize] = '\0';
    if (fsize != NULL) {*fsize = rsize;}
  }

  fclose( rfile );
  return (void*)rdata;
}

bool R_Init( res_id_t* err_res_id ) {
  puts( "init resources subsystem..." );

  for( unsigned int i = 0; i < res_count; ++i ) {
    res_info[i].data = load_file( res_files[i], &(res_info[i].len) );
    if (res_info[i].data == NULL) {
      eprintf( "error: failed to load resource #%u from file \"%s\"\n",
        i, res_files[i] );
      if (err_res_id != NULL) { *err_res_id = i; }
      return false;
    }
  }

  return true;
}

void R_Free() {
  for( unsigned int i = 0; i < res_count; ++i ) {
    free( res_info[i].data );
    res_info[i] = NO_RESOURCE;
  }
}

resource_t R_GetResource( res_id_t id ) {
  if ((id < 0) || (id >= res_count)) {
    eprintf( "error: invalid resource id #%d\n", id );
    return NO_RESOURCE;
  }
  return res_info[id];
}


