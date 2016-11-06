#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>

#define TIMER_RESOLUTION 1.0

#define X_TIMERS \
  X( TIMER_SHOW, 1 ) \
  X( TIMER_FPS, 30 ) \
  X( TIMER_UPS, 60 )

typedef enum {
  #define X(id, ...) id,
  X_TIMERS
  #undef X
} timer_id_t;

extern bool T_UpdateState( timer_id_t id );
extern int T_GetLastCount( timer_id_t id );
extern void T_UpdateCounters();

#endif // __TIMER_H__
