#include "autoReloadTimer.h"
#include "stdint.h"
#include "sound.h"
#include "trigger.h"

#define RELOAD_TIMER_EXPIRE_VALUE 300000
#define INVALID_STATE "invalid_st\n"

static uint16_t reloadCounter;

enum lockoutTimer_st_t { init_st, waiting_st, running_st };
static enum lockoutTimer_st_t currentState;

// Need to init things.
void autoReloadTimer_init() {
    currentState = init_st;
}

// Calling this starts the timer.
void autoReloadTimer_start() {
    currentState = running_st;
}

// Returns true if the timer is currently running.
bool autoReloadTimer_running() {
    return currentState == running_st;
}

// Disables the autoReloadTimer and re-initializes it.
void autoReloadTimer_cancel() {
    currentState = waiting_st;
}

// Standard tick function.
void autoReloadTimer_tick() {
    // Perfom State transitions
  switch (currentState) {
    case init_st:
        currentState = waiting_st;
        break;
    case waiting_st:
        break;
    case running_st:
        // Timer has reached 3 seconds
        if (reloadCounter >= RELOAD_TIMER_EXPIRE_VALUE) {
            currentState = waiting_st;
            reloadCounter = 0;
            sound_playSound(sound_gunReload_e);
            trigger_setRemainingShotCount(LAST_STANDING_AMMO);
        }
        break;
    default:
        DPRINTF(INVALID_STATE);
        break;
  }

  // Perform State Actions
  switch (currentState) {
    case init_st:
        break;
    case waiting_st:
        break;
    case running_st:
        reloadCounter++;
        break;
    default:
        DPRINTF(INVALID_STATE);
        break;
  }
}