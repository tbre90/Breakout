#ifndef PLATFORM_H

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

#define PLATFORM_H
#endif
