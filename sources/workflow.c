#include "workflow.h"

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

#include "error.h"
#include "render.h"
#include "timer.h"
#include "logic.h"

#define DEFAULT_MOUSE_X 0.0
#define DEFAULT_MOUSE_Y 0.0

#define MAIN_WND_TITLE "Hello OpenGL"

#if defined(__cplusplus)
  #define UNUSED(x)
#elif defined(__GNUC__)
  #define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
  #define UNUSED(x) /*@unused@*/ x
#else
  #define UNUSED(x) UNUSED_##x
#endif

static GLFWwindow* main_wnd;

static void GLFW_CB_keyboard( GLFWwindow* UNUSED(window),
  int key, int UNUSED(scancode), int action, int UNUSED(mods)
) {
  const bool state = action != GLFW_RELEASE;
  #define MAP_STATE_TO_KEY(_key, _state) \
    case _key: key_st[_state] = state; break;
  switch (key) {
    MAP_STATE_TO_KEY( GLFW_KEY_W, GO_FORWARD );
    MAP_STATE_TO_KEY( GLFW_KEY_S, GO_BACKWARD );
    MAP_STATE_TO_KEY( GLFW_KEY_A, STEP_LEFT );
    MAP_STATE_TO_KEY( GLFW_KEY_D, STEP_RIGHT );
    MAP_STATE_TO_KEY( GLFW_KEY_MINUS, AMBIENT_LESS );
    MAP_STATE_TO_KEY( GLFW_KEY_EQUAL, AMBIENT_MORE );
    MAP_STATE_TO_KEY( GLFW_KEY_LEFT_BRACKET, DIFFUSE_LESS );
    MAP_STATE_TO_KEY( GLFW_KEY_RIGHT_BRACKET, DIFFUSE_MORE );
    /*
    MAP_STATE_TO_KEY( GLFW_KEY_Q, TURN_LEFT );
    MAP_STATE_TO_KEY( GLFW_KEY_E, TURN_RIGHT );
    */
  }
  #undef MAP_STATE_TO_KEY

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose( main_wnd, true );
        break;
      case GLFW_KEY_R:
        Rotating = !Rotating;
        break;
      case GLFW_KEY_T:
        NormalMapping = !NormalMapping;
        break;
    }
  }
}

void GLFW_CB_mouse_pos( GLFWwindow* UNUSED(window),
  double xpos, double ypos
) {
  static double prev_xpos = DEFAULT_MOUSE_X;
  static double prev_ypos = DEFAULT_MOUSE_Y;
  mouse_off.xoff = xpos - prev_xpos;
  mouse_off.yoff = prev_ypos - ypos;
  prev_xpos = xpos;
  prev_ypos = ypos;
  mouse_off.update = true;
}

bool main_init() {
  puts( "init window and input..." );

  if (!glfwInit()) {
    eputs( "error: glfwInit() failed." );
    return false;
  }

  glfwWindowHint( GLFW_RESIZABLE, false );
  main_wnd = glfwCreateWindow( WINDOW_SIZE_W, WINDOW_SIZE_H,
    MAIN_WND_TITLE, NULL, NULL );
  if (main_wnd == NULL) {
    eputs( "error: failed to create main window." );
    return false;
  }
  glfwMakeContextCurrent( main_wnd ); //must be placed before glewInit()!
  glfwSwapInterval(0); //fuck off vsync

  glfwSetCursorPos( main_wnd, DEFAULT_MOUSE_X, DEFAULT_MOUSE_Y );
  glfwSetInputMode( main_wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

  glfwSetKeyCallback( main_wnd, &GLFW_CB_keyboard );
  glfwSetCursorPosCallback( main_wnd, &GLFW_CB_mouse_pos );

  return true;
}

bool main_loop() {
  puts( "entering main loop..." );

  while ( !glfwWindowShouldClose( main_wnd ) ) {

    if (T_UpdateState(TIMER_FPS)) {
      G_RenderFrame();
      glfwSwapBuffers( main_wnd );
    } else if (T_UpdateState(TIMER_UPS)) {
      L_Update();
    } else {
      glfwPollEvents();
    }

    if (T_UpdateState(TIMER_SHOW)) {
      T_UpdateCounters();
      char title[256];
      sprintf( title, MAIN_WND_TITLE" (FPS: %d; UPS: %d)",
        T_GetLastCount(TIMER_FPS), T_GetLastCount(TIMER_UPS) );
      glfwSetWindowTitle( main_wnd, title );
    }
  }

  return true;
}

void main_free() {
  glfwTerminate();
}
