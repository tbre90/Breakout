#ifndef GAME_H

#include "..\Include\common.h"
#include "..\Include\platform_api.h"
#include "..\Include\platform_sound_api.h"

#define GET_RANDOM_COLOR(c, cutoff) \
    (c) = ((rand() % (cutoff) << 16) |\
           (rand() % (cutoff) << 8) |\
           (rand() % (cutoff)));

#define GAME_OVER_TEXT_SWAP_TIME 1000

#define FPS_60 16.6666666666667L
#define FPS_144 6.94444444444444L

// type structs
struct rectangle
{
    int x;
    int y;
    int width;
    int height;
    int color;
};

struct window
{
    int x;
    int y;
    int width;
    int height;
};

struct paddle
{
    struct rectangle rect;
    int velocity;
};

struct brick
{
    struct rectangle rect;
    int alive;
};

struct bricks
{
    struct brick *bricks;
    int row;
    int column;
    size_t num_alive;
};

struct circle
{
    struct rectangle circle;
    int velocity;

    // traveling directions
    unsigned char up;
    unsigned char down;
    unsigned char left;
    unsigned char right;
};

struct entities
{
    struct paddle paddle;
    struct circle ball;
    struct bricks bricks;
};

struct text
{
    int width;
    int height;
};

struct game
{
    struct window window;
    struct entities entities;
    struct text text;
    enum { GAME_OVER = 0, GAME_RUNNING, GAME_WON, BRICK_REMOVED, } state;
    double ms_per_frame;
    int    rng_seeded;
};

static struct paddle
create_paddle(int x,
              int y,
              int width,
              int height,
              int velocity,
              int paddle_color);

static int
move_paddle(struct paddle * const paddle,
            struct platform_data const * const pd);

static int
is_paddle_out_of_bounds(struct paddle * const paddle,
                        struct window const * const window);

static struct circle
create_unsexy_non_antialiased_ball(
        int x,
        int y,
        int width,
        int height,
        int velocity,
        int ball_color);

static int
move_ball(struct circle * const ball);

static enum game_state
check_for_ball_collision(struct entities * const entities,
                         struct window const * const window);

static struct bricks*
create_bricks(int width,
              int height,
              int rows,
              int columns,
              int padding,
              struct bricks *const dest);

static int
make_sexy_tint(int const color);

static int
draw_paddle(struct paddle const * const paddle);

static int
draw_ball(struct circle const * const ball);

static int
draw_bricks(struct bricks const * const bricks);

#define GAME_H
#endif
