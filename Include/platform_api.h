#ifndef PLATFORM_API_H

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
    #if defined(_UNICODE) || defined(UNICODE)
    #define GET_WORKING_DIR(buffer, maxlen) _wgetcwd((buffer), (maxlen))
    #else
    #define GET_WORKING_DIR(buffer, maxlen) _getcwd((buffer), (maxlen))
    #endif
#define PATH_MAX 260
#else
#include <unistd.h>
#define GET_WORKING_DIR(buffer, maxlen) getcwd((buffer), (maxlen))
#define PATH_MAX 4096
#endif

void
platform_request_window_dimensions(int *width, int *height);

int
platform_set_background(int color);

int
platform_paint_background(void);

int
platform_draw_filled_rect(int x, int y, int width, int height, int fill);

int
platform_draw_rect_frame(int x, int y, int width, int height, int tint);

int
platform_remove_rectangle(int x, int y, int width, int height);

int
platform_draw_tinted_circle(int x, int y, int width, int height, int fill, int tint);

int
platform_remove_circle(int x, int y, int width, int height);

// TODO: make more general
//int
//platform_sound(void);

#define PLATFORM_API_H
#endif
