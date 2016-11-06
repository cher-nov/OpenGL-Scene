#ifndef __RENDER_H__
#define __RENDER_H__

#include <stdbool.h>

#define WINDOW_SIZE_W 800
#define WINDOW_SIZE_H 600
#define WINDOW_ASPECT ((double)(WINDOW_SIZE_W)/(double)(WINDOW_SIZE_H))

#define RGB1(c) ((c)/255.0f)
#define RGB3(r,g,b) (const float[]){RGB1(r), RGB1(g), RGB1(b)}
#define RGB4(r,g,b) (const float[]){RGB1(r), RGB1(g), RGB1(b), 1.0f}
#define RGBA(r,g,b,a) (const float[]){RGB1(r), RGB1(g), RGB1(b), RGB1(a)}

extern bool G_Init();
extern void G_Free();
extern void G_RenderFrame();

#endif // __RENDER_H__
