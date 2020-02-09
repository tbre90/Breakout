#ifndef COMMON_H

struct platform_data
{
    struct movement
    {
        int mouse_enabled;
        int mouse_x;

        int keyboard_left;
        int keyboard_right;
    } movement;
};


#define COMMON_H
#endif
