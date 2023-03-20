#ifndef _NATIVE_PLAYBACK_H
#define _NATIVE_PLAYBACK_H

#include <stdint.h>
#include <stdbool.h>

void native_playback_init(void);
void native_playback_statemachine(uint8_t event, void *arg);
bool native_playback_action_play(void);
bool native_playback_action_pause(void);
bool native_playback_action_next(void);
bool native_playback_action_prev(void);
bool user_check_native_playback_allowable(uint8_t event);

#endif  // _NATIVE_PLAYBACK_H
