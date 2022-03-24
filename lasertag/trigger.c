#include "trigger.h"
#include "buttons.h"
#include "filter.h"
#include "mio.h"
#include "switches.h"
#include "transmitter.h"
#include "sound.h"

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define GUN_TRIGGER_PRESSED 1
#define DEBOUNCE_TIMER_MAX_VALUE 5000
#define RELOAD_TIMER_MAX_VALUE 300000
#define TEST_SHOT_COUNT 1000

#define INVALID_ST_MSG "INVALID STATE\n"
#define INIT_ST_MSG "trigger_init_st\n"
#define WAITING_ST_MSG "trigger_waiting_st\n"
#define TRIGGER_ST_MSG "trigger_st\n"
#define TRIGGER_PRESS_ST_MSG "trigger_press_st\n"
#define TRIGGER_RELEASE_ST_MSG "trigger_release_st\n"
#define TRIGGER_RELOAD_ST_MSG "trigger_reload_st\n"
#define DOWN_CHAR 'D'
#define UP_CHAR 'U'
#define NEWLINE '\n'

// #define DEBUG
#if defined(DEBUG)
#include "xil_printf.h"
#include <stdio.h>
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

enum trigger_st_t {
  init_st,
  waiting_st,
  trigger_st,
  trigger_press_st,
  trigger_release_st
};
static enum trigger_st_t currentState;

static bool ignoreGunInput = false;
static uint16_t debounceTimer, reloadTimer;
static trigger_shotsRemaining_t shotsRemaining;

// This is a debug state print routine. It will print the names of the states
// each time tick() is called. It only prints states if they are different than
// the previous state.
void trigger_debugStatePrint() {
  static enum trigger_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state
  // name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false; // previousState will be defined, firstPass is false.
    previousState =
        currentState;       // keep track of the last state that you were in.
    switch (currentState) { // This prints messages based upon the state that
                            // you were in.
    case init_st:
      DPRINTF(INIT_ST_MSG);
      break;
    case waiting_st:
      DPRINTF(WAITING_ST_MSG);
      break;
    case trigger_st:
      DPRINTF(TRIGGER_ST_MSG);
      break;
    case trigger_press_st:
      DPRINTF(TRIGGER_PRESS_ST_MSG);
      break;
    case trigger_release_st:
      DPRINTF(TRIGGER_RELEASE_ST_MSG);
      break;
    }
  }
}

// Trigger can be activated by either btn0 or the external gun that is attached
// to TRIGGER_GUN_TRIGGER_MIO_PIN Gun input is ignored if the gun-input is high
// when the init() function is invoked.
bool triggerPressed() {
  return ((!ignoreGunInput &
           (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) ||
          (buttons_read() & BUTTONS_BTN0_MASK));
}

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion
// in lab web pages). Initializes the mio subsystem.
void trigger_init() {
  mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
  debounceTimer = 0;
  shotsRemaining = TEST_SHOT_COUNT;

  // If the trigger is pressed when trigger_init() is called, assume that the
  // gun is not connected and ignore it.
  if (triggerPressed()) {
    ignoreGunInput = true;
  }
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() { ignoreGunInput = false; }

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() { ignoreGunInput = true; }

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
  return shotsRemaining;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
  shotsRemaining = count;
}

// Standard tick function.
void trigger_tick() {
  trigger_debugStatePrint();
  // Perform state transitions
  switch (currentState) {
  case init_st:
    currentState = waiting_st;
    break;
  case waiting_st:
    // Trigger pressed, debounce button unless no shots remaining
    if (triggerPressed() && shotsRemaining > 0 && !ignoreGunInput) {
      currentState = trigger_st;
      shotsRemaining--;
    }
    else if (triggerPressed() && shotsRemaining == 0 && !ignoreGunInput) {
      sound_playSound(sound_gunClick_e);
    }
    break;
  case trigger_st:
    // Trigger released before debounce
    // If trigger is debounced, transmit signal and wait for release
    if (!triggerPressed()) {
      currentState = waiting_st;
    } else if (debounceTimer >= DEBOUNCE_TIMER_MAX_VALUE) {
      currentState = trigger_press_st;
      reloadTimer = 0;
      DPCHAR(DOWN_CHAR);
      DPCHAR(NEWLINE);
      transmitter_run();
      sound_playSound(sound_gunFire_e);
    }
    break;
  case trigger_press_st:
    // Trigger released, begin debouncing or reload gun
    if (!triggerPressed()) {
      debounceTimer = 0;
      currentState = trigger_release_st;
    }
    else if (reloadTimer >= RELOAD_TIMER_MAX_VALUE) {
      sound_playSound(sound_gunReload_e);
      trigger_setRemainingShotCount(LAST_STANDING_AMMO);
      currentState = trigger_release_st;
    }
    break;
  case trigger_release_st:
    // Release is debounced
    if (debounceTimer >= DEBOUNCE_TIMER_MAX_VALUE) {
      DPCHAR(UP_CHAR);
      DPCHAR(NEWLINE);
      currentState = waiting_st;
    }
    break;
  default:
    DPRINTF(INVALID_ST_MSG);
    break;
  }

  // Perform state actions
  switch (currentState) {
  case init_st:
    break;
  case waiting_st:
    debounceTimer = 0;
    break;
  case trigger_st:
    debounceTimer++;
    break;
  case trigger_press_st:
    reloadTimer++;
    break;
  case trigger_release_st:
    debounceTimer++;
    break;
  default:
    DPRINTF(INVALID_ST_MSG);
    break;
  }
}

// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest() {
  transmitter_enableTestMode();
  trigger_enable();
  trigger_setRemainingShotCount(TEST_SHOT_COUNT);

  // Run the test until button 1 is pressed
  while (!(buttons_read() & BUTTONS_BTN1_MASK)) {
    uint16_t frequency = switches_read() % FILTER_FREQUENCY_COUNT;
    transmitter_setFrequencyNumber(frequency);
  }

  trigger_disable();
}