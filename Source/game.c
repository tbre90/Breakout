#include <stdlib.h>
#include <time.h>

#include "..\Include\game.h"

static struct game game;

static int rect_intersect(struct rectangle *r1, struct rectangle *r2);

static int sound_init = 0;

int game_initialize(void)
{
    if (sound_init == 0)
    {
        sound_init = init_sound_system();
        if (!sound_init)
        { sound_init = -1; }
    }

    if (!game.rng_seeded)
    {
        srand((unsigned)time(NULL));
        game.rng_seeded = 1;
    }

    if (game.entities.bricks.bricks)
    {
        free(game.entities.bricks.bricks);
        game.entities.bricks.bricks = NULL;
    }

    int window_width = 0;
    int window_height = 0;

    platform_request_window_dimensions(&window_width, &window_height);

    game.window.x = 0;
    game.window.y = 0;
    game.window.width = window_width;
    game.window.height = window_height;

    int paddle_width = window_width / 20;
    int paddle_height = window_height / 40;
    int paddle_acceleration = window_width / 80;

    game.entities.paddle =
        create_paddle(
            (window_width / 2) - (paddle_width / 2),
            window_height - (paddle_height * 2),
            paddle_width,
            paddle_height,
            paddle_acceleration,
            0x000000
        );

    int ball_radius = paddle_width / 3;
    int ball_acceleration = window_height / 80;

    game.entities.ball =
        create_unsexy_non_antialiased_ball(
            (window_width / 2) - (ball_radius / 2),
            (window_height / 2) - (ball_radius / 2),
            ball_radius,
            ball_radius,
            ball_acceleration,
            0x000000
        );

    int brick_height = window_height / 40;
    int brick_width = window_width / 20;

    int num_brick_columns = window_width / brick_width;
    int num_brick_rows = 8;

    int padding = 2;

    if (!create_bricks(
        brick_width,
        brick_height,
        num_brick_rows,
        num_brick_columns,
        padding,
        &game.entities.bricks
    ))
    {
        return 0;
    }

    platform_set_background(0xFFFFFF);
    platform_paint_background();

    game.ms_per_frame = FPS_60;

    return 1;
}

double
game_request_ms_per_frame(void)
{
    return game.ms_per_frame;
}

int
game_main(struct keyboard *keyboard)
{
    keyboard = keyboard;
    // move paddle and check if it's out of bounds
    platform_remove_rectangle(
        game.entities.paddle.rect.x,
        game.entities.paddle.rect.y,
        game.entities.paddle.rect.width,
        game.entities.paddle.rect.height
    );
    move_paddle(&game.entities.paddle, keyboard);
    is_paddle_out_of_bounds(&game.entities.paddle, &game.window);

    // move ball and check for collisions
    platform_remove_circle(
        game.entities.ball.circle.x,
        game.entities.ball.circle.y,
        game.entities.ball.circle.width,
        game.entities.ball.circle.height
    );
    move_ball(&game.entities.ball);
    check_for_ball_collision(&game.entities, &game.window);

    draw_ball(
        &game.entities.ball
    );

    draw_paddle(
        &game.entities.paddle
    );

    draw_bricks(
        &game.entities.bricks
    );

    return 1;
}


// (x) will be the x coordinate of the first brick
// caller's responsibility to make sure number of bricks
// fit in window
static struct bricks*
create_bricks(int width,
              int height,
              int rows,
              int columns,
              int padding,
              struct bricks * const dest)
{

    dest->bricks = calloc(rows * columns, sizeof(struct brick));
    if (!(dest->bricks))
    {
        return NULL;
    }

    for (int i = 0; i < rows; ++i)
    {
        // make fabulous bricks
        // (or not, who knows, the color is random)
        int random_color;
        GET_RANDOM_COLOR(random_color, 256);

        for (int j = 0; j < columns; ++j)
        {
            int x = (j * width) + padding;
            int y = (i * height) + padding;

            dest->bricks[(i * columns) + j].rect.x = x;
            dest->bricks[(i * columns) + j].rect.width = width - padding;
            dest->bricks[(i * columns) + j].rect.y = y;
            dest->bricks[(i * columns) + j].rect.height = height - padding;
            dest->bricks[(i * columns) + j].rect.color = random_color;
            dest->bricks[(i * columns) + j].alive = 1;
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
            struct keyboard const * const keyboard)
{
    if (keyboard->left) 
    { 
        paddle->rect.x -= paddle->velocity;
    }

    if (keyboard->right)
    {
        paddle->rect.x += paddle->velocity;
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
    }
    
    // handle ball colliding with left side of window
    else if (ball->circle.x <= window->x)
    {
        int ball_oob_left = ball->circle.x - window->x;
        ball->circle.x -= ball_oob_left;
        ball->left = 0;
        ball->right = 1;
    }

    // handle ball colliding with right side of window
    else if ((ball->circle.x + ball->circle.width) >= (window->x + window->width))
    {
        int ball_oob_right = (ball->circle.x + ball->circle.width) - (window->x + window->width);
        ball->circle.x -= ball_oob_right;
        ball->right = 0;
        ball->left = 1;
    }

    // handle ball colliding with top of window
    else if (ball->circle.y <= window->y)
    {
        int ball_oob_top = ball->circle.y - window->y;
        ball->circle.y -= ball_oob_top;
        ball->up = 0;
        ball->down = 1;
    }

    // handle ball colliding with paddle
    else if (rect_intersect(&(paddle->rect), &(ball->circle)))
    {
        int adjustment = (ball->circle.y + ball->circle.height) - paddle->rect.y;
        ball->circle.y -= adjustment;

        ball->down = 0;
        ball->up = 1;
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

                    goto end;
                }
            }
        }
    }

end:
    return GAME_RUNNING;
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
