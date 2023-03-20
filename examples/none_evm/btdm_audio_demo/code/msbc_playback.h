#ifndef _MSBC_PLAYBACK_H
#define _MSBC_PLAYBACK_H

#include <stdint.h>
#include <stdbool.h>

void msbc_playback_init(void);
void msbc_playback_statemachine(uint8_t event, void *arg);
bool msbc_playback_action_play(void);
bool msbc_playback_action_pause(void);
bool msbc_playback_action_next(void);

#endif  // _MSBC_PLAYBACK_H
