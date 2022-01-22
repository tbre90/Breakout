#ifndef GAME_API_H
#define GAME_API_H

int game_initialize(void);

int game_main(struct platform_data *keyboard);

void game_window_resize(void);

double game_request_ms_per_frame(void);

#endif
