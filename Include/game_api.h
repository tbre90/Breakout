#ifndef GAME_API_H
#define GAME_API_H

int game_initialize(void);

int game_main(struct keyboard *keyboard);

double game_request_ms_per_frame(void);

#endif
