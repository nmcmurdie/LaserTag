#include "transmitter.h"
#include "filter.h"
#include "mio.h"
#include "buttons.h"
#include "switches.h"
#include "utils.h"
#include "stdio.h"

#define TRANSMITTER_PULSE_WIDTH 2000
#define TRANSMITTER_PULSE_TEST_WIDTH 200
#define INVALID_STATE "INVALID STATE\n"
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define BOUNCE_DELAY 5

#define INIT_ST_MSG "init_st\n"
#define WAITING_ST_MSG "waiting_st\n"
#define TRANSMIT_LOW_ST_MSG "transmit_low\n"
#define TRANSMIT_HIGH_ST_MSG "transmit_high\n"

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

enum transmitter_st_t {
    init_st,
    waiting_st,
    transmit_low_st,
    transmit_high_st
};
static enum transmitter_st_t currentState;

static uint16_t freqNum, transmitTimer, transmitCounter, tickTime, transmitCounterLength;
static bool isContinuous = false, debugPrint = false;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint() {
  static enum transmitter_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        DPRINTF(INIT_ST_MSG);
        break;
      case waiting_st:
        DPRINTF(WAITING_ST_MSG);
        break;
      case transmit_low_st:
        DPRINTF(TRANSMIT_LOW_ST_MSG);
        break;
      case transmit_high_st:
        DPRINTF(TRANSMIT_HIGH_ST_MSG);
        break;
     }
  }
}

// Set the output JF-1 pin to value
void transmitter_set_jf1(uint8_t value) {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, value);
}

// Standard init function.
void transmitter_init() {
    mio_init(false);
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);

    currentState = init_st;
    transmitTimer = 0;
    transmitCounter = 0;
    transmitCounterLength = TRANSMITTER_PULSE_WIDTH;
    tickTime = 0;
}

// Starts the transmitter.
void transmitter_run() {
    currentState = transmit_high_st;
    transmitter_set_jf1(TRANSMITTER_HIGH_VALUE);
}

// Returns true if the transmitter is still running.
bool transmitter_running() {
    return currentState == transmit_low_st || currentState == transmit_high_st;
}

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    if (isContinuous || !transmitter_running()) {
        // Only set frequency if transmitting is continuous or not running
        freqNum = frequencyNumber;
        tickTime = filter_frequencyTickTable[frequencyNumber] / 2;
    }
}

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() {
    return freqNum;
}

// Standard tick function.
void transmitter_tick() {
    // Perform state transitions
    switch (currentState) {
        case init_st:
            currentState = waiting_st;
            break;
        case waiting_st:
            break;
        case transmit_high_st:
            DPRINTF("1");
            if (!isContinuous && transmitCounter >= transmitCounterLength) {
                // Reached 200ms
                currentState = waiting_st;
                transmitter_set_jf1(TRANSMITTER_LOW_VALUE);
            }
            else if (transmitTimer == tickTime) {
                // Timer transition from high to low
                currentState = transmit_low_st;
                transmitTimer = 0;
                transmitCounter += tickTime;
                transmitter_set_jf1(TRANSMITTER_LOW_VALUE);
                DPRINTF("\n");
            }
            break;
        case transmit_low_st:
            DPRINTF("0");
            if (!isContinuous && transmitCounter >= transmitCounterLength) {
                // Reached 200ms
                currentState = waiting_st;
                transmitter_set_jf1(TRANSMITTER_LOW_VALUE);
            }
            else if (transmitTimer == tickTime) {
                // Timer transition from low to high
                currentState = transmit_high_st;
                transmitTimer = 0;
                transmitCounter += tickTime;
                transmitter_set_jf1(TRANSMITTER_HIGH_VALUE);
                DPRINTF("\n");
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
            transmitCounter = 0;
            transmitTimer = 0;
            break;
        case transmit_high_st:
            transmitTimer++;
            break;
        case transmit_low_st:
            transmitTimer++;
            break;
        default:
            DPRINTF(INVALID_STATE);
            break;
    }
}

//sets counterlength to a shorter value to decrease operating time
void transmitter_enableTestMode() {
    transmitCounterLength = TRANSMITTER_PULSE_TEST_WIDTH;
}

//sets counterlength to normal 
void transmitter_disableTestMode() {
    transmitCounterLength = TRANSMITTER_PULSE_WIDTH;
}

// Tests the transmitter.
void transmitter_runTest() {
    DPRINTF("starting transmitter_runTest()\n");
    mio_init(false);
    buttons_init();                                         // Using buttons
    switches_init();                                        // and switches.
    transmitter_init();                                     // init the transmitter.
    transmitter_enableTestMode();                           // Prints diagnostics to stdio.
    while (!(buttons_read() & BUTTONS_BTN1_MASK)) {         // Run continuously until BTN1 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
        transmitter_run();                                    // Start the transmitter.
        while (transmitter_running()) {                       // Keep ticking until it is done.
        transmitter_tick();                                 // tick.
        utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
        }
        DPRINTF("\ncompleted one test period.\n");
    }
    transmitter_disableTestMode();
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    DPRINTF("exiting transmitter_runTest()\n");
}

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise,
// transmits one pulse-width and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is in
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until the last 200 ms pulse is complete. NOTE: while running continuously,
// the transmitter will change frequencies at the end of each 200 ms pulse.
void transmitter_setContinuousMode(bool continuousModeFlag) {
    isContinuous = continuousModeFlag;
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN1 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
void transmitter_runNoncontinuousTest() {
    transmitter_init();                           //init the transmitter
    transmitter_run();                            //starts the transmitter
    transmitter_setContinuousMode(false);         //sets continuous flag to false  
    while(!(buttons_read() & BUTTONS_BTN1_MASK)){ //Run until button 1 is pressed
        while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                               // tick.
        }
    }
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes to the changes in the slide switches.
// Test runs until BTN1 is pressed.
void transmitter_runContinuousTest() {
    transmitter_init();                             //init the transmitter
    transmitter_run();                              //starts the transmitter
    transmitter_setContinuousMode(true);            //sets continuous flag to true
    while(1){                                       //runs forever until stopped
        while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                               // tick.
        }
    }
}