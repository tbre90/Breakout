#ifndef PLATFORM_API_H

#if defined(_WIN32) || defined(_WIN64)
#define PATH_MAX 260
#else
#define PATH_MAX 4096
#endif

int
get_working_dir(char * const buffer, int buff_len);

char*
folder_exists(char * const path, char const * const folder, size_t path_len);

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

int
platform_put_text(int x, int y, char const * const text, size_t length);

void
platform_request_text_dimension(int * const width, int * const height);

#define PLATFORM_API_H
#endif
