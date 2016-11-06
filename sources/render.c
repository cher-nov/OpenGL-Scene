#include "render.h"

#include <stdarg.h>
#include <math.h>
#include <GLEW/glew.h>
#include <SOIL/SOIL.h>

#include "error.h"
#include "resource.h"
#include "logic.h"
#include "lintrans.h"
#include "routines.h"

#ifndef LINMATH_H_ROW_MAJOR
  #define MAT4_ROWS GL_FALSE
#else
  #define MAT4_ROWS GL_TRUE
#endif

#define TEXTURE_FLAGS \
  ( SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB )
#define NORMALMAP_FLAGS \
  ( SOIL_FLAG_INVERT_Y )

#define GRID_LINES 20
#define GRID_VERTICES (GRID_LINES*4)
#define GRID_RADIUS 5.0f

typedef struct {
  vec3 pos; //position
  vec2 tex; //texture coords
  vec3 norm; //normal
  vec3 tang; //tangent
} vertex_t;

static bool do_free = false;

static struct {
  struct { GLuint VAO, tex_gfx, tex_map; } triangle;
  struct { GLuint VAO; } grid;
} obj = {GL_NONE};

static struct {
  struct {
    GLuint ID;
    struct { GLuint Vertex, Fragment; } sh; //linked shaders
    struct { //uniform variables
      GLuint gMW;
      GLuint gMWVP;
      GLuint gNormalMx;
      GLuint NormalMapping;
      struct { GLuint Color, AmbientIntensity, Direction,
        DiffuseIntensity; } gDirLight;
    } uf;
  } Render;
} prog = {GL_NONE};

/* =================================================================== */

static GLuint shader_from_res( res_id_t id, GLenum type ) {
  resource_t res = R_GetResource(id);
  if (res.data == NULL) { return GL_NONE; }

  GLuint shader_obj = glCreateShader(type);
  glShaderSource( shader_obj, 1, (const GLchar* const*)&res.data, (GLint*)&res.len );
  glCompileShader( shader_obj );

  GLint success;
	glGetShaderiv( shader_obj, GL_COMPILE_STATUS, &success );

	if (!success) {
    eprintf( "%s: compilation error\n", __func__ );

    GLint msg_l;
    glGetShaderiv( shader_obj, GL_INFO_LOG_LENGTH, &msg_l );
    if (msg_l > 0) {
      GLchar info[msg_l];
      glGetShaderInfoLog( shader_obj, msg_l, NULL, info );
      eprintf( "%s\n", info );
    }

    glDeleteShader( shader_obj );
    return GL_NONE;
	}

  return shader_obj;
}

static GLuint texture_from_res( res_id_t id, unsigned int load_flags ) {
  resource_t res = R_GetResource(id);
  if (res.data == NULL) { return GL_NONE; }

  GLuint tex = SOIL_load_OGL_texture_from_memory( res.data, res.len,
    SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, load_flags );

  if (tex == GL_NONE) {
    eprintf( "error creating texture from resource #%d (%s)",
      id, res_files[id] );
    return GL_NONE;
  }

  return tex;
}

static GLuint prog_from_shaders( int count, ... ) {
  GLuint prog = glCreateProgram();

  va_list args;
  va_start( args, count );
  for( int i = 0; i < count; ++i ) {
    glAttachShader( prog, va_arg( args, GLuint ) );
  }
  va_end( args );

  glLinkProgram( prog );
  GLint success;
  glGetProgramiv( prog, GL_LINK_STATUS, &success );

  if (!success) {
    eprintf( "%s: linkage error\n", __func__ );

    GLint msg_l;
    glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &msg_l );
    if (msg_l > 0) {
      GLchar info[msg_l];
      glGetProgramInfoLog( prog, msg_l, NULL, info );
      eprintf( "%s\n", info );
    }

    glDeleteProgram( prog );
    return GL_NONE;
  }

  return prog;
}

/* =================================================================== */

//calculates normal for the triangle surface, defined by 3 vectors
static void calc_normal( vec3 result, vec3 vx0, vec3 vx1, vec3 vx2 ) {
  vec3 edge1, edge2;
  vec3_sub( edge1, vx1, vx0 );
  vec3_sub( edge2, vx2, vx0 );

  vec3_mul_cross( result, edge1, edge2 );
  vec3_norm( result, result );
}

//calculates tangent from specified triangle and it's texture coords
static void calc_tangent( vec3 result,
  vec3 pos0, vec3 pos1, vec3 pos2, vec2_t tex0, vec2_t tex1, vec2_t tex2 )
{
  vec3_t edge1, edge2;
  vec3_sub( edge1.v, pos1, pos0 );
  vec3_sub( edge2.v, pos2, pos0 );

  float deltaU1 = tex1.i.x - tex0.i.x;
  float deltaV1 = tex1.i.y - tex0.i.y;
  float deltaU2 = tex2.i.x - tex0.i.x;
  float deltaV2 = tex2.i.y - tex0.i.y;

  float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

  //prevents from corrupted tangents when surface has zero texture coords
  //that provokes a division by zero error, and 1.0f / 0.0f == INF
  if (isinf(f)) {f = 0.0f;}

  vec3_t tangent;
  tangent.i.x = f * (deltaV2 * edge1.i.x - deltaV1 * edge2.i.x);
  tangent.i.y = f * (deltaV2 * edge1.i.y - deltaV1 * edge2.i.y);
  tangent.i.z = f * (deltaV2 * edge1.i.z - deltaV1 * edge2.i.z);

  vec3_dup( result, tangent.v );
}

//this calculates normals and tangents for specified vertices
static void calc_vertex_subinfo( vertex_t* vertices, int vertex_num,
  GLuint indices[][3], int index_num )
{
  //this initializes subinfo with zeros or erases previous data
  for( int i = 0; i < vertex_num; ++i ) {
    vec3_dup( vertices[i].norm, VEC3(0.0f,0.0f,0.0f) );
    vec3_dup( vertices[i].tang, VEC3(0.0f,0.0f,0.0f) );
  }

  for( int i = 0; i < index_num; ++i ) {
    int id0 = indices[i][0];
    int id1 = indices[i][1];
    int id2 = indices[i][2];

    vertex_t* vx0 = &vertices[id0];
    vertex_t* vx1 = &vertices[id1];
    vertex_t* vx2 = &vertices[id2];

    vec3 normal;
    calc_normal( normal, vx0->pos, vx1->pos, vx2->pos );

    vec3 tangent;
    calc_tangent( tangent, vx0->pos, vx1->pos, vx2->pos,
      cVEC2T(vx0->tex), cVEC2T(vx1->tex), cVEC2T(vx2->tex) );

    //interpolating normals
    vec3_add( vx0->norm, vx0->norm, normal );
    vec3_add( vx1->norm, vx1->norm, normal );
    vec3_add( vx2->norm, vx2->norm, normal );

    //interpolating tangents
    vec3_add( vx0->tang, vx0->tang, tangent );
    vec3_add( vx1->tang, vx1->tang, tangent );
    vec3_add( vx2->tang, vx2->tang, tangent );
  }

  for( int i = 0; i < vertex_num; ++i ) {
    vec3_norm( vertices[i].norm, vertices[i].norm );
    vec3_norm( vertices[i].tang, vertices[i].tang );
  }
}

/* =================================================================== */

static void init_VAO_triangle() {
  GLuint VAO, VBO, IBO; //IBO aka EBO
  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  const int stride = sizeof(vertex_t) / sizeof(float);
  vertex_t vx_triangle[] = {
    //positions and texture coords
    //normals and tangents are left uninitialized
    { .pos = {-1.0f, -1.0f,  0.5f}, .tex = {0.0f, 0.0f} },
    { .pos = { 0.0f, -1.0f, -1.0f}, .tex = {0.5f, 0.0f} },
    { .pos = { 1.0f, -1.0f,  0.5f}, .tex = {1.0f, 0.0f} },
    { .pos = { 0.0f,  1.0f,  0.0f}, .tex = {0.5f, 1.0f} }
  };
  //note that the order of indices is important for normals calculation
  GLuint id_triangle[][3] = { {0, 3, 1}, {1, 3, 2}, {2, 3, 0}, {1, 2, 0} };
  calc_vertex_subinfo( vx_triangle, NUMOFE(vx_triangle),
    id_triangle, NUMOFE(id_triangle) );

  glGenBuffers( 1, &VBO );
  glBindBuffer( GL_ARRAY_BUFFER, VBO );
  glBufferData( GL_ARRAY_BUFFER,
    sizeof(vx_triangle), vx_triangle, GL_STATIC_DRAW );

  glGenBuffers( 1, &IBO );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER,
    sizeof(id_triangle), id_triangle, GL_STATIC_DRAW );

  //position attribute
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE,
    stride * sizeof(GLfloat), (GLvoid*)0 );
  glEnableVertexAttribArray(0);

  //texture coords attribute
  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE,
    stride * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)) );
  glEnableVertexAttribArray(1);

  //surface normal attribute
  glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE,
    stride * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)) );
  glEnableVertexAttribArray(2);

  //surface tangent attribute
  glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE,
    stride * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)) );
  glEnableVertexAttribArray(3);

  obj.triangle.VAO = VAO;
}

static void init_VAO_grid() {
  GLuint VAO, VBO; //IBO aka EBO
  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  glGenBuffers( 1, &VBO );
  vec3 vx_grid[GRID_VERTICES];
  for( int i = 0; i < GRID_VERTICES; i += 4 ) {
    float step = (GRID_RADIUS*2.0f) / (float)(GRID_LINES-1) * (i/4);
    vec3_dup( vx_grid[i+0], VEC3(GRID_RADIUS-step, -1.0f, -GRID_RADIUS) );
    vec3_dup( vx_grid[i+1], VEC3(GRID_RADIUS-step, -1.0f,  GRID_RADIUS) );
    vec3_dup( vx_grid[i+2], VEC3(-GRID_RADIUS, -1.0f, GRID_RADIUS-step) );
    vec3_dup( vx_grid[i+3], VEC3( GRID_RADIUS, -1.0f, GRID_RADIUS-step) );
  }

  glBindBuffer( GL_ARRAY_BUFFER, VBO );
  glBufferData( GL_ARRAY_BUFFER,
    sizeof(vx_grid), vx_grid, GL_STATIC_DRAW );

  //position attributes
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
  glEnableVertexAttribArray(0);

  obj.grid.VAO = VAO;
}

/* =================================================================== */

static bool init_objects() {
  RETURN_FALSE_IF_EQUAL(
    obj.triangle.tex_gfx = texture_from_res( RES_TEXTURE_GRAPHIC, TEXTURE_FLAGS ),
    GL_NONE );
  RETURN_FALSE_IF_EQUAL(
    obj.triangle.tex_map = texture_from_res( RES_TEXTURE_NORMALS, NORMALMAP_FLAGS ),
    GL_NONE );

  init_VAO_triangle();
  init_VAO_grid();

  return true;
}

static bool init_programs() {
  RETURN_FALSE_IF_EQUAL( prog.Render.sh.Vertex =
    shader_from_res( RES_SHADER_VERTEX, GL_VERTEX_SHADER ),
    GL_NONE );
  RETURN_FALSE_IF_EQUAL( prog.Render.sh.Fragment =
    shader_from_res( RES_SHADER_FRAGMENT, GL_FRAGMENT_SHADER ),
    GL_NONE );

  RETURN_FALSE_IF_EQUAL( prog.Render.ID =
    prog_from_shaders( 2, prog.Render.sh.Vertex, prog.Render.sh.Fragment ),
    GL_NONE );

  #define REGISTER_UNIFORM(root, name) \
    root.name = glGetUniformLocation( prog.Render.ID, #name )
  REGISTER_UNIFORM( prog.Render.uf, gMW );
  REGISTER_UNIFORM( prog.Render.uf, gMWVP );
  REGISTER_UNIFORM( prog.Render.uf, gNormalMx );
  REGISTER_UNIFORM( prog.Render.uf, NormalMapping );
  REGISTER_UNIFORM( prog.Render.uf, gDirLight.Color );
  REGISTER_UNIFORM( prog.Render.uf, gDirLight.AmbientIntensity );
  REGISTER_UNIFORM( prog.Render.uf, gDirLight.Direction );
  REGISTER_UNIFORM( prog.Render.uf, gDirLight.DiffuseIntensity );
  #undef REGISTER_UNIFORM

  glUseProgram( prog.Render.ID );
  glUniform1i( glGetUniformLocation( prog.Render.ID, "TexGFX" ), 0 );
  glUniform1i( glGetUniformLocation( prog.Render.ID, "TexMap" ), 1 );
  glUseProgram( GL_NONE );

  return true;
}

static void get_transform( mat4x4 mxMW, mat4x4 mxMWVP ) {
  ltSetScale( 1.0f, 1.0f, 1.0f );
  ltSetWorldPos( 0.0f, 0.0f, 0.0f );
  ltSetRotate( 0.0f, RotateY, 0.0f );
  ltSetCamera( camera.position.v, camera.target.v, camera.up.v );
  ltSetPerspective( WINDOW_ASPECT, 45.0f, 1.0f, 100.0f );

  ltGetMW( mxMW );
  ltGetWVP( mxMWVP );
  mat4x4_mul( mxMWVP, mxMWVP, mxMW );
}

/* =================================================================== */

bool G_Init() {
  puts( "init renderer..." );

  GLenum init_glew = glewInit();
  if (init_glew != GLEW_OK) {
    eprintf( "error: glewInit() failed: %s\n", glewGetErrorString( init_glew ) );
    return false;
  }
  do_free = true;

  glEnable( GL_DEPTH_TEST );

  RETURN_IF_FALSE( init_objects() );
  RETURN_IF_FALSE( init_programs() );
  return true;
}

void G_Free() {
  if (!do_free) { return; }

  // do we really need this? we don't free shaders, VBO etc. anyway
  glDeleteTextures( 1, &obj.triangle.tex_gfx );
  glDeleteTextures( 1, &obj.triangle.tex_map );
}

void G_RenderFrame() {
  const float ambient_coff = light.ambient / (float)LIGHT_INTENSITY_UNIT;
  const float diffuse_coff = light.diffuse / (float)LIGHT_INTENSITY_UNIT;

  vec3_t heaven;
  vec3_dup( heaven.v, light.color );
  vec3_scale( heaven.v, heaven.v, ambient_coff + 0.3f );
  glClearColor( heaven.i.x, heaven.i.y, heaven.i.z, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glUseProgram( prog.Render.ID );

  mat4x4 model, trans, model_inv, normal_mx;
  get_transform( model, trans );

  // for normals we must inverse and then transpose our model-world matrix
  // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
  mat4x4_invert( model_inv, model );
  mat4x4_transpose( normal_mx, model_inv );

  glUniformMatrix4fv( prog.Render.uf.gMW, 1, MAT4_ROWS, &model[0][0] );
  glUniformMatrix4fv( prog.Render.uf.gMWVP, 1, MAT4_ROWS, &trans[0][0] );
  glUniformMatrix4fv( prog.Render.uf.gNormalMx, 1, MAT4_ROWS, &normal_mx[0][0] );

  glUniform3fv( prog.Render.uf.gDirLight.Color, 1, light.color );
  glUniform1f( prog.Render.uf.gDirLight.AmbientIntensity, ambient_coff );
  glUniform3fv( prog.Render.uf.gDirLight.Direction, 1, light.direction );
  glUniform1f( prog.Render.uf.gDirLight.DiffuseIntensity, diffuse_coff );

  glUniform1i( prog.Render.uf.NormalMapping, NormalMapping );

  //glActiveTexture( GL_NONE );
  //glBindTexture( GL_TEXTURE_2D, GL_NONE );
  glBindVertexArray( obj.grid.VAO );
  glDrawArrays( GL_LINES, 0, GRID_VERTICES );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, obj.triangle.tex_gfx );
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, obj.triangle.tex_map );
  glBindVertexArray( obj.triangle.VAO );
  glDrawElements( GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0 );
}
