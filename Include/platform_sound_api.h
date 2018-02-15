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
test_sound(char const * const file);

#endif
