#include "mio.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include "leds.h"
#include "buttons.h"
#include "switches.h"
#include "stdio.h"

#define INVALID_STATE "INVALID STATE\n"

#define HIGH 1
#define LOW 0
#define LED_MASK_OFF 0x0

static bool enable;
static uint16_t hitLedTimer;


enum hitLedTimer_st_t {
    init_st,
    waiting_st,
    running_st
};
static enum hitLedTimer_st_t currentState;

// Calling this starts the timer.
void hitLedTimer_start(){

}

// Returns true if the timer is currently running.
bool hitLedTimer_running(){

}

// Standard tick function.
void hitLedTimer_tick(){
    //Perform state transitions
    switch (currentState){
        case init_st:
            currentState=waiting_st;
            break;
        case waiting_st:
            if(enable){
                currentState=running_st;
                hitLedTimer_turnLedOn();
            }
            break;
        case running_st:
            if(hitLedTimer==LOCKOUT_TIMER_EXPIRE_VALUE){
                currentState=waiting_st;
                hitLedTimer_turnLedOff();
                hitLedTimer=0;
            }
            break;
        default:
            printf(INVALID_STATE);
            break;
    }

    //perform state actions
    switch (currentState){
        case init_st:
            break;
        case waiting_st:
            break;
        case running_st:
            hitLedTimer++;
            break;
        default:
            printf(INVALID_STATE);
            break;
    }

}

// Need to init things.
void hitLedTimer_init(){
    mio_init(false);
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
    hitLedTimer=0;
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn(){
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN,  HIGH);
    leds_write(SWITCHES_SW0_MASK);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff(){
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN,  LOW);
    leds_write(LED_MASK_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable(){
    enable = false;
}

// Enables the hitLedTimer.
void hitLedTimer_enable(){
    enable = true;
}

// Runs a visual test of the hit LED.
// The test continuously blinks the hit-led on and off.
void hitLedTimer_runTest(){
    hitLedTimer_init();
    hitLedTimer_enable();
    while(!(buttons_read() & BUTTONS_BTN1_MASK)){
        hitLedTimer_start();
    }
}