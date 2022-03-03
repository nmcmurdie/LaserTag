#include "lockoutTimer.h"
#include "intervalTimer.h"
#include "stdint.h"
#include "stdio.h"
#include "utils.h"

#define INIT_ST_MSG "init_st"
#define WAITING_ST_MSG "waitin_st"
#define RUNNING_ST_MSG "running_st"
#define INVALID_STATE "INVALID STATE\n"

#define TIMER_LENGTH_S 0.500
#define TIMER_ERROR_S 0.005
#define SHORT_DELAY 1

#define DEBUG
#if defined(DEBUG)
#include "xil_printf.h"
#include <stdio.h>
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

static uint16_t lockOutCounter;

enum lockoutTimer_st_t { init_st, waiting_st, running_st };
static enum lockoutTimer_st_t currentState;

// Calling this starts the timer.
void lockoutTimer_start() { currentState = running_st; }

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
  lockOutCounter = 0;
  currentState = init_st;
}

// Returns true if the timer is running.
bool lockoutTimer_running() { return currentState == running_st; }

// Standard tick function.
void lockoutTimer_tick() {
  // Perfom State transitions
  switch (currentState) {
  case init_st:
    currentState = waiting_st;
    break;
  case waiting_st:
    break;
  case running_st:
    // Timer has reached 1/2 second
    if (lockOutCounter >= LOCKOUT_TIMER_EXPIRE_VALUE) {
      currentState = waiting_st;
      lockOutCounter = 0;
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
    lockOutCounter++;
    break;
  default:
    DPRINTF(INVALID_STATE);
    break;
  }
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
  intervalTimer_init(INTERVAL_TIMER_TIMER_1);

  lockoutTimer_start();
  intervalTimer_start(INTERVAL_TIMER_TIMER_1);

  while (lockoutTimer_running())
    utils_msDelay(SHORT_DELAY);
  intervalTimer_stop(INTERVAL_TIMER_TIMER_1);

  double duration =
      intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_1);
  DPRINTF("Total duration: %f\n", duration);

  return (duration > TIMER_LENGTH_S - TIMER_ERROR_S) &&
         (duration < TIMER_LENGTH_S + TIMER_ERROR_S);
}