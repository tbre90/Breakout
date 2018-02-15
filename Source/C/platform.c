#include "..\..\Include\main.h"

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
