#include "mio.h"
#include "hitLedTimer.h"
#include "leds.h"
#include "buttons.h"
#include "switches.h"
#include "stdio.h"
#include "utils.h"

#define INVALID_STATE "INVALID STATE\n"
#define INIT_ST_MSG "init_st\n"
#define WAITING_ST_MSG "waiting_st\n"
#define RUNNING_ST_MSG "running_st\n"

#define HIGH 1
#define LOW 0
#define LED_MASK_OFF 0x0

#define DEBUG
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

static bool enable;
static uint16_t hitLedTimer;


enum hitLedTimer_st_t {
    init_st,
    waiting_st,
    running_st
};
static enum hitLedTimer_st_t currentState;

// Calling this starts the timer.
void hitLedTimer_start() {
    enable = true;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return currentState == running_st;
}

// Standard tick function.
void hitLedTimer_tick() {
    // Perform state transitions
    switch (currentState) {
        case init_st:
            currentState = waiting_st;
            break;
        case waiting_st:
            if (enable) {
                // Run timer and turn on LED
                currentState = running_st;
                hitLedTimer_turnLedOn();
            }
            break;
        case running_st:
            if (hitLedTimer >= HIT_LED_TIMER_EXPIRE_VALUE) {
                // Timer has expired, turn off LED
                currentState = waiting_st;
                hitLedTimer_turnLedOff();
                hitLedTimer = 0;
                enable = false;
            }
            break;
        default:
            DPRINTF(INVALID_STATE);
            break;
    }

    // Perform state actions
    switch (currentState) {
        case init_st:
            break;
        case waiting_st:
            break;
        case running_st:
            hitLedTimer++;
            break;
        default:
            DPRINTF(INVALID_STATE);
            break;
    }

}

// Need to init things.
void hitLedTimer_init() {
    mio_init(false);
    leds_init(false);
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
    hitLedTimer = 0;
    enable = false;
    currentState = init_st;
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN,  HIGH);
    leds_write(SWITCHES_SW0_MASK);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN,  LOW);
    leds_write(LED_MASK_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable() {
    enable = false;
}

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    enable = true;
}

// Runs a visual test of the hit LED.
// The test continuously blinks the hit-led on and off.
void hitLedTimer_runTest() {
    hitLedTimer_init();
    
    while (1) {
        // Run timer loop
        hitLedTimer_start();
        while (hitLedTimer_running()) utils_msDelay(1);
        utils_msDelay(300);
    }
}