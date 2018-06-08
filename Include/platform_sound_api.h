#ifndef PLATFORM_SOUND_API_H
#define PLATFORM_SOUND_API_H

#ifdef __cplusplus
extern "C"
#endif
int
init_sound_system(void);

#ifdef __cplusplus
extern "C"
#endif
void
play_sound(int sound);

#ifdef __cplusplus
extern "C"
#endif
int
load_sound(char const * const file);

#ifdef __cplusplus
extern "C"
#endif
int
load_sound_embedded(char const * const byte_array, size_t data_size);

#endif
