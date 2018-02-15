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

#if defined(_WIN32) || defined(_WIN64)
#else
    #if defined(_UNICODE) || defined(UNICODE)
    #define FOLDER_EXISTS(folder, t_ref) \
        do \
        { \
            LPWIN32_FIND_DATAW find_file; \
            HANDLE h = FindFirstFile(L"" folder, &find_file); \
            if (h == INVALID_HANDLE_VALUE) { *t_ref = 0; } \
            else \
            { \
                CloseHandle(h); \
            }
        } while (0)
    #else
    #endif
#error "folder_exists not yet implemented for linux/unix"
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
