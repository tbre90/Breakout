#ifndef PLATFORM_SOUND_API_H
#define PLATFORM_SOUND_API_H

#ifdef _cplusplus
extern "C"
#endif
int
init_sound_system(void);

#ifdef _cplusplus
extern "C"
#endif
void
test_sound(char const * const file);

#endif
