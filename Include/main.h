#ifndef MAIN_H

void
request_window_dimensions(int *width, int *height);

int
set_background(int color);

int
paint_background(void);

int
draw_filled_rect(int x, int y, int width, int height, int fill);

int
draw_rect_frame(int x, int y, int width, int height, int tint);

int
remove_rectangle(int x, int y, int width, int height);

int
draw_circle(int x, int y, int width, int height, int fill, int tint);

#define MAIN_H
#endif
