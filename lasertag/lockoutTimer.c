#include "lockoutTimer.h"
#include "stdint.h"
#include "stdio.h"

#define INIT_ST_MSG "init_st"
#define WAITING_ST_MSG "waitin_st"
#define RUNNING_ST_MSG "running_st"

#define INVALID_STATE "INVALID STATE\n"

static uint16_t lockOutCounter;


enum lockoutTimer_st_t {
    init_st,
    waiting_st,
    running_st
};
static enum lockoutTimer_st_t currentState;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
/*void debugStatePrint() {
  static enum lockoutTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf(INIT_ST_MSG);
        break;
      case waiting_st:
        printf(WAITING_ST_MSG);
        break;
      case running_st:
        printf(RUNNING_ST_MSG);
        break;
     }
  }
}*/

// Calling this starts the timer.
void lockoutTimer_start(){
    currentState=running_st;
}

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init(){
    lockOutCounter=0;
    currentState=init_st;
}

// Returns true if the timer is running.
bool lockoutTimer_running(){
    return currentState==running_st;
}

// Standard tick function.
void lockoutTimer_tick(){
    //Perfom State transitions
    switch(currentState){
        case init_st:
            currentState=waiting_st;
            break;
        case waiting_st:
            break;
        case running_st:
            if(lockOutCounter==LOCKOUT_TIMER_EXPIRE_VALUE){
                //timer has reached 1/2 second
                currentState=waiting_st;
            }
            break;
        default:
            printf(INVALID_STATE);
            break;
    }

    //Perform State Actions
    switch (currentState)
    {
    case init_st:
        break;
    case waiting_st:
        break;
    case running_st:
        lockOutCounter++;       //increment the counter by 1;
        break;
    default:
        printf(INVALID_STATE);
        break;
    }
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest(){

}