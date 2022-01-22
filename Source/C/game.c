#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "..\..\Include\game.h"
#include "..\..\Include\game_api.h"
#include "..\..\Include\asset.h"

static struct game g_game;
static int         sound_init = 0;
static int         g_ball_sound = 0;
static int         g_bkcolor = 0xFFFFFF;

static void get_entity_defaults(enum entity_type es, struct entity_default *ed);
static int rect_intersect(struct rectangle *r1, struct rectangle *r2);

enum entity_type { entity_ball = 0, entity_paddle, entity_brick, entity_brick_container };

struct entity_default
{
    union
    {
        struct circle ball;
        struct paddle paddle;
        struct brick brick;
        struct bricks bricks;
    };
};

int game_initialize(void)
{
    if (sound_init == 0)
    {
        sound_init = init_sound_system();
        if (!sound_init)
        { sound_init = -1; }
        else
        {
            g_ball_sound = load_sound_embedded(game_asset_sound_ball_hit, game_asset_sound_ball_hit_size);
        }
    }

    if (!g_game.rng_seeded)
    {
        srand((unsigned)time(NULL));
        g_game.rng_seeded = 1;
    }

    int window_width = 0;
    int window_height = 0;

    platform_request_window_dimensions(&window_width, &window_height);

    g_game.window.x = 0;
    g_game.window.y = 0;
    g_game.window.width = window_width;
    g_game.window.height = window_height;

    struct entity_default ed = {0};

    get_entity_defaults(entity_paddle, &ed);
    g_game.entities.paddle =
        create_paddle(
            ed.paddle.rect.x,
            ed.paddle.rect.y,
            ed.paddle.rect.width,
            ed.paddle.rect.height,
            ed.paddle.velocity,
            ed.paddle.rect.color
        );

    get_entity_defaults(entity_ball, &ed);
    g_game.entities.ball =
        create_unsexy_non_antialiased_ball(
            ed.ball.circle.x,
            ed.ball.circle.y,
            ed.ball.circle.width,
            ed.ball.circle.height,
            ed.ball.velocity,
            ed.ball.circle.color
        );
    
    get_entity_defaults(entity_brick_container, &ed);

    struct bricks bricks = ed.bricks;

    if (!g_game.entities.bricks.bricks)
    {
        g_game.entities.bricks.bricks = calloc((size_t)bricks.row * (size_t)bricks.column, sizeof(struct brick));
        if (!g_game.entities.bricks.bricks)
        { return 0; }
    }
    else
    {
        memset(g_game.entities.bricks.bricks, 0, sizeof(struct brick) * bricks.row * bricks.column);
    }

    get_entity_defaults(entity_brick, &ed);

    create_bricks(
        ed.brick.rect.width,
        ed.brick.rect.height,
        bricks.row,
        bricks.column,
        ed.brick.padding,
        &g_game.entities.bricks
    );

    platform_request_text_dimension(
        &g_game.text.width,
        &g_game.text.height
    );

    g_game.ms_per_frame = FPS_144;
    g_game.state = GAME_RUNNING;

    platform_set_background(g_bkcolor);
    platform_paint_background();

    return 1;
}

static void
get_entity_defaults(enum entity_type es, struct entity_default *ed)
{
    const int paddle_width          = g_game.window.width / 20;
    const int paddle_height         = g_game.window.height / 40;
    const int paddle_acceleration   = g_game.window.width / (80 * 2);

    const int ball_radius           = paddle_width / 3;
    const int ball_acceleration     = g_game.window.height / (80 * 2);
    const int ball_color            = 0x000000;

    const int brick_padding         = 2;
    const int brick_height          = g_game.window.height / 40;
    const int brick_width           = g_game.window.width / 20;
    const int bricks_rows           = 8;
    const int bricks_columns        = g_game.window.width / brick_width;

    const int window_width          = g_game.window.width;
    const int window_height         = g_game.window.height;
    const int window_x_mid          = g_game.window.width / 2;
    const int window_y_mid          = g_game.window.height / 2;

    switch (es)
    {
        case entity_ball:
        {
            ed->ball.circle.x      = window_x_mid - (ball_radius / 2);
            ed->ball.circle.y      = window_y_mid - (ball_radius / 2);
            ed->ball.circle.height = ball_radius;
            ed->ball.circle.width  = ball_radius;
            ed->ball.velocity      = ball_acceleration;
            ed->ball.circle.color  = ball_color;
        } break;

        case entity_paddle:
        {

            ed->paddle.rect.x       = window_x_mid - (paddle_width / 2);
            ed->paddle.rect.y       = window_height - (paddle_height * 2);
            ed->paddle.rect.width   = paddle_width;
            ed->paddle.rect.height  = paddle_height;
            ed->paddle.rect.color   = 0x000000;
            ed->paddle.velocity     = paddle_acceleration;
        } break;

        case entity_brick:
        {
            ed->brick.alive         = 1;
            ed->brick.padding       = brick_padding;
            ed->brick.rect.color    = 0x000000;
            ed->brick.rect.x        = 0;
            ed->brick.rect.y        = 0;
            ed->brick.rect.width    = brick_width;
            ed->brick.rect.height   = brick_height;
        } break;

        case entity_brick_container:
        {
            ed->bricks.row          = bricks_rows;
            ed->bricks.column       = bricks_columns;
            ed->bricks.bricks       = NULL;
            ed->bricks.num_alive    = 0;
        } break;
    }
}

void
game_window_resize(void)
{
    platform_set_background(g_bkcolor);
    platform_paint_background();
}

double
game_request_ms_per_frame(void)
{
    return g_game.ms_per_frame;
}

int
game_main(struct platform_data *pd)
{
    if (g_game.state == GAME_OVER)
    {
        char const * const game_over_text = "GAME OVER";
        size_t length = strlen(game_over_text);

        platform_put_text(
            g_game.window.width / 2 - (int)length / 2,
            g_game.window.height / 2 - g_game.text.height / 2,
            game_over_text,
            length
        );
    }
    else if (g_game.state == GAME_WON)
    {
        char const * const game_won_text = "GAME WON";
        size_t length = strlen(game_won_text);
        platform_put_text(
            g_game.window.width / 2 - (int)length / 2,
            g_game.window.height / 2 - g_game.text.height / 2,
            game_won_text,
            length
        );
    }
    else
    {
        // move paddle and check if it's out of bounds
        platform_remove_rectangle(
            g_game.entities.paddle.rect.x,
            g_game.entities.paddle.rect.y,
            g_game.entities.paddle.rect.width,
            g_game.entities.paddle.rect.height
        );
        move_paddle(&g_game.entities.paddle, pd);
        is_paddle_out_of_bounds(&g_game.entities.paddle, &g_game.window);

        // move ball and check for collisions
        platform_remove_circle(
            g_game.entities.ball.circle.x,
            g_game.entities.ball.circle.y,
            g_game.entities.ball.circle.width,
            g_game.entities.ball.circle.height
        );
        move_ball(&g_game.entities.ball);
        enum state s = check_for_ball_collision(&g_game.entities, &g_game.window);
        if (s == BRICK_REMOVED)
        {
            g_game.entities.bricks.num_alive--;
            if (!g_game.entities.bricks.num_alive)
            {
                g_game.state = GAME_WON;
            }
        }
        else
        { g_game.state = s; }

        draw_ball(
            &g_game.entities.ball
        );

        draw_paddle(
            &g_game.entities.paddle
        );

        draw_bricks(
            &g_game.entities.bricks
        );
    }

    return 1;
}

static struct bricks*
create_bricks(int width,
              int height,
              int rows,
              int columns,
              int padding,
              struct bricks * const dest)
{
    for (int i = 0; i < rows; ++i)
    {
        // make fabulous bricks
        // (or not, who knows, the color is random)
        int random_color;
        GET_RANDOM_COLOR(random_color, 256);

        for (int j = 0; j < columns; ++j)
        {
            int x = (j * width) + padding;
            int y = (height * 4) + (i * height) + padding;

            dest->bricks[(i * columns) + j].rect.x = x;
            dest->bricks[(i * columns) + j].rect.width = width - padding;
            dest->bricks[(i * columns) + j].rect.y = y;
            dest->bricks[(i * columns) + j].rect.height = height - padding;
            dest->bricks[(i * columns) + j].rect.color = random_color;
            dest->bricks[(i * columns) + j].alive = 1;
            dest->num_alive++;
        }
    }

    dest->row = rows;
    dest->column = columns;

    return dest;
}

static struct paddle
create_paddle(int x,
              int y,
              int width,
              int height,
              int velocity,
              int paddle_color)
{
    struct paddle new_paddle;
    new_paddle.rect.color    = paddle_color;
    new_paddle.rect.x        = x;
    new_paddle.rect.y        = y;
    new_paddle.rect.width    = width;
    new_paddle.rect.height   = height;
    new_paddle.velocity      = velocity;

    return new_paddle;
}

static int
move_paddle(struct paddle * const paddle,
            struct platform_data const * const pd)
{
    if (pd->movement.mouse_enabled)
    {
        int mouse_position = pd->movement.mouse_x;
        int mid_of_paddle = paddle->rect.x + (paddle->rect.width / 2);

        int deadzone_start = mouse_position - paddle->velocity;
        int deadzone_end   = mouse_position + paddle->velocity;

        if (mid_of_paddle < deadzone_start)
        {
            paddle->rect.x += paddle->velocity;
        }
        else if (mid_of_paddle > deadzone_end)
        {
            paddle->rect.x -= paddle->velocity;
        }

#ifdef DEBUG_CURSOR_MOVEMENT
        char buffer[128];

        snprintf(buffer, 128, "Mouse position: %d\n", mouse_position);
        OutputDebugStringA(buffer);

        snprintf(buffer, 128, "Paddle middle: %d\n", mid_of_paddle);
        OutputDebugStringA(buffer);
#endif

    }
    else
    {
        if (pd->movement.keyboard_left)
        {
            paddle->rect.x -= paddle->velocity;
        }
        if (pd->movement.keyboard_right)
        {
            paddle->rect.x += paddle->velocity;
        }
    }

    return 1;
}

static int
is_paddle_out_of_bounds(struct paddle * const paddle,
                        struct window const * const window)
{
    if (paddle->rect.x < 0)
    {
        int oob_left = 0 - paddle->rect.x;

        paddle->rect.x += oob_left;
    }
    else if ((paddle->rect.x + paddle->rect.width) > (window->x + window->width))
    {
        int oob_right = (paddle->rect.x + paddle->rect.width) - (window->x + window->width);

        paddle->rect.x -= oob_right;
    }

    return 1;
}

static struct circle
create_unsexy_non_antialiased_ball(
        int x,
        int y,
        int width,
        int height,
        int velocity,
        int ball_color)
{
    struct circle ball       = {0};
    ball.circle.color        = ball_color;
    ball.circle.x            = x;
    ball.circle.y            = y;
    ball.circle.width        = width;
    ball.circle.height       = height;
    ball.velocity        = velocity;

    // initial travel direction
    ball.down = 1;

    unsigned char *left = &ball.left;
    unsigned char *right = &ball.right;
    unsigned char *lr_array[2];
    lr_array[0] = left;
    lr_array[1] = right;

    int index = rand() % 2;

    *(lr_array[index]) = 1;

    return ball;
}

static int
move_ball(struct circle * const ball)
{
    if (ball->up) 
    { 
        ball->circle.y -= ball->velocity;
    }
    if (ball->down)
    {
        ball->circle.y += ball->velocity;
    }
    if (ball->left)
    {
        ball->circle.x -= ball->velocity;
    }
    if (ball->right)
    {
        ball->circle.x += ball->velocity;
    }

    return 1;
}

static enum game_state
check_for_ball_collision(struct entities * const entities,
                         struct window const * const window)
{
    struct circle *ball = &(entities->ball);
    struct paddle *paddle = &(entities->paddle);
    struct bricks *bricks = &(entities->bricks);

    // handle ball colliding with bottom of window
    if ((ball->circle.y + ball->circle.height) >= (window->y + window->height))
    {
        int ball_oob_bottom = (ball->circle.y + ball->circle.height) - (window->y + window->height);
        ball->circle.y -= ball_oob_bottom;
        ball->down = 0;
        ball->up = 1;
        play_sound(g_ball_sound);

#ifndef TESTING_BUILD
        return GAME_OVER;
#endif
    }
    
    // handle ball colliding with left side of window
    else if (ball->circle.x <= window->x)
    {
        int ball_oob_left = ball->circle.x - window->x;
        ball->circle.x -= ball_oob_left;
        ball->left = 0;
        ball->right = 1;
        play_sound(g_ball_sound);
    }

    // handle ball colliding with right side of window
    else if ((ball->circle.x + ball->circle.width) >= (window->x + window->width))
    {
        int ball_oob_right = (ball->circle.x + ball->circle.width) - (window->x + window->width);
        ball->circle.x -= ball_oob_right;
        ball->right = 0;
        ball->left = 1;
        play_sound(g_ball_sound);
    }

    // handle ball colliding with top of window
    else if (ball->circle.y <= window->y)
    {
        int ball_oob_top = ball->circle.y - window->y;
        ball->circle.y -= ball_oob_top;
        ball->up = 0;
        ball->down = 1;
        play_sound(g_ball_sound);
    }

    // handle ball colliding with paddle
    else if (rect_intersect(&(paddle->rect), &(ball->circle)))
    {
        int adjustment = (ball->circle.y + ball->circle.height) - paddle->rect.y;
        ball->circle.y -= adjustment;

        ball->down = 0;
        ball->up = 1;
        play_sound(g_ball_sound);
    }

    // handle colliding with bricks
    else
    {
        for (int row = 0; row < bricks->row; ++row)
        {
            for (int col = 0; col < bricks->column; ++col)
            {
                struct brick *b = &(bricks->bricks[(row * bricks->column) + col]);

                if (b->alive && rect_intersect(&(b->rect), &(ball->circle)))
                {
                    int adjustment = (b->rect.y + b->rect.height) - ball->circle.y;
                    ball->circle.y += adjustment;

                    b->alive = 0;
                    ball->up = 0;
                    ball->down = 1;

                    platform_remove_rectangle(
                        bricks->bricks[(row * bricks->column) + col].rect.x,
                        bricks->bricks[(row * bricks->column) + col].rect.y,
                        bricks->bricks[(row * bricks->column) + col].rect.width,
                        bricks->bricks[(row * bricks->column) + col].rect.height
                    );

                    play_sound(g_ball_sound);
                    goto brick_removed;
                }
            }
        }
    }

return GAME_RUNNING;

brick_removed:
    return BRICK_REMOVED;
}

static int
make_sexy_tint(int const color)
{
    int red   = color & 0x000000FF;
    int green = color & 0x0000FF00;
    int blue  = color & 0x00FF0000;

    red   += (int)((float)(255 - red) * 0.25f);
    green += (int)((float)(255 - green) * 0.25f);
    blue  += (int)((float)(255 - blue) * 0.25f);

    return red | green | blue;
}

static int
draw_paddle(struct paddle const * const paddle)
{
    platform_draw_filled_rect(
        paddle->rect.x,
        paddle->rect.y,
        paddle->rect.width,
        paddle->rect.height,
        paddle->rect.color
    );

    platform_draw_rect_frame(
        paddle->rect.x,
        paddle->rect.y,
        paddle->rect.width,
        paddle->rect.height,
        make_sexy_tint(paddle->rect.color)
    );

    return 1;
}

static int
draw_ball(struct circle const * const ball)
{
    platform_draw_tinted_circle(
            ball->circle.x,
            ball->circle.y,
            ball->circle.width,
            ball->circle.height,
            ball->circle.color,
            make_sexy_tint(ball->circle.color)
    );

    return 1;
}

static int
draw_bricks(struct bricks const * const bricks)
{
    for (int row = 0; row < bricks->row; row++)
    {
        for (int col = 0; col < bricks->column; col++)
        {
            if (bricks->bricks[(row * bricks->column) + col].alive)
            {
                platform_draw_filled_rect(
                    bricks->bricks[(row * bricks->column) + col].rect.x,
                    bricks->bricks[(row * bricks->column) + col].rect.y,
                    bricks->bricks[(row * bricks->column) + col].rect.width,
                    bricks->bricks[(row * bricks->column) + col].rect.height,
                    bricks->bricks[(row * bricks->column) + col].rect.color
                );

                platform_draw_rect_frame(
                    bricks->bricks[(row * bricks->column) + col].rect.x,
                    bricks->bricks[(row * bricks->column) + col].rect.y,
                    bricks->bricks[(row * bricks->column) + col].rect.width,
                    bricks->bricks[(row * bricks->column) + col].rect.height,
                    make_sexy_tint(bricks->bricks[(row * bricks->column) + col].rect.color)
                );
            }
        }
    }

    return 1;
}

static int
rect_intersect(struct rectangle *r1, struct rectangle *r2)
{
    return !(r2->x > (r1->x + r1->width)
            || (r2->x + r2->width) < r1->x
            || r2->y > (r1->y + r1->height)
            || (r2->y + r2->height) < r1->y);
}
