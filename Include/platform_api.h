#ifndef PLATFORM_API_H

#if defined(_WIN32) || defined(_WIN64)
#define PATH_MAX 260
#else
#define PATH_MAX 4096
#endif

int
get_working_dir(char *buffer, int buff_len);

int
folder_exists(char *folder);

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
