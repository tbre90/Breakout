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

int
put_text(int x, int y, char const * const text, size_t length);

void
request_text_dimension(int * const width, int * const height);

#define MAIN_H
#endif
