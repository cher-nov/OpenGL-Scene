#include "timer.h"

#include <GLFW/glfw3.h>

typedef double timer_val_t;

#define X(...) +1
static const int timer_count[X_TIMERS] = {
  #undef X
  #define X(id,count) count,
  X_TIMERS
};
#undef X

#define X(...) +1
static const timer_val_t time_delta[X_TIMERS] = {
  #undef X
  #define X(id,count) (TIMER_RESOLUTION / (timer_val_t)count),
  X_TIMERS
};
#undef X

#define X(...) +1
static timer_val_t last_time[X_TIMERS] = {0.0};
static int last_count[X_TIMERS] = {0};
static int new_count[X_TIMERS] = {0};
#undef X

bool T_UpdateState( timer_id_t id ) {
  timer_val_t cur_time = glfwGetTime();
  if ( (cur_time-last_time[id]) >= time_delta[id] ) {
    new_count[id]++;
    if ( new_count[id] < timer_count[id] ) {
      last_time[id] += time_delta[id];
    } else {
      last_time[id] = cur_time;
    }
    return true;
  }
  return false;
}

int T_GetLastCount( timer_id_t id ) {
  return last_count[id];
}

void T_UpdateCounters() {
  #define X(id,...) \
    last_count[id] = new_count[id]; \
    new_count[id] = 0;
  X_TIMERS
  #undef X
}


