#include <string.h>

#include "..\..\Include\main.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <direct.h>
#define GET_WORKING_DIR(buffer, maxlen) _getcwd((buffer), (maxlen))
#define PATH_MAX 260
#else
#include <unistd.h>
#define GET_WORKING_DIR(buffer, maxlen) getcwd((buffer), (maxlen))
#define PATH_MAX 4096
#endif

int
get_working_dir(char *buffer, int buff_len)
{
    if (!GET_WORKING_DIR(buffer, buff_len))
    {
        return 0;
    }

    return 1;
}

int
folder_exists(char *folder)
{
#if defined(_WIN32) || defined(_WIN64)
    char dir_name[PATH_MAX] = {0};

    if (!get_working_dir(dir_name, PATH_MAX))
    { goto no_folder; }

    strncat(dir_name, "\\*", 4);

    WIN32_FIND_DATAA ffd = {0};
    HANDLE find_handle = FindFirstFileA(dir_name, &ffd);

    if (find_handle == INVALID_HANDLE_VALUE)
    { goto no_folder; }

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (!strncmp(ffd.cFileName, folder, strlen(folder)))
            { goto found_folder; }
        }
    } while (FindNextFile(find_handle, &ffd) != 0);

    FindClose(find_handle);

#else
#error "folder_exists not yet implemented for linux/unix"
#endif

no_folder:
    return 0;

found_folder:
    return 1;
}

void
platform_request_window_dimensions(int *width, int *height)
{
    request_window_dimensions(width, height);
}

int
platform_set_background(int color)
{
    return set_background(color);
}

int
platform_paint_background(void)
{
    return paint_background();
}

int
platform_draw_filled_rect(int x, int y, int width, int height, int fill)
{
    return draw_filled_rect(x, y, width, height, fill);
}

int
platform_draw_rect_frame(int x, int y, int width, int height, int tint)
{
    return draw_rect_frame(x, y, width, height, tint);
}

int
platform_remove_rectangle(int x, int y, int width, int height)
{
    return remove_rectangle(x, y, width, height);
}

int
platform_draw_tinted_circle(int x, int y, int width, int height, int fill, int tint)
{
    return draw_circle(x, y, width, height, fill, tint);
}

int
platform_remove_circle(int x, int y, int width, int height)
{
    return remove_rectangle(x, y, width, height);
}


// Need to make this more general
// figure out which sound api to use
//int platform_sound(void)
//{
//}
