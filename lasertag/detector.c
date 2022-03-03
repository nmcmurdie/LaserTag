#include "detector.h"
#include "filter.h"

static detector_hitCount_t hitCounts[FILTER_FREQUENCY_COUNT];
static bool ignoredFreq[FILTER_FREQUENCY_COUNT];
static uint32_t fudgeFactor = 0;
static int16_t hitFrequency = -1;
static bool ignoreHits = false;

// Always have to init things.
// bool array is indexed by frequency number, array location set for true to
// ignore, false otherwise. This way you can ignore multiple frequencies.
void detector_init(bool ignoredFrequencies[]) {
    // Clear hit counts and copy over ignored frequencies
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
        hitCounts[i] = 0;
        ignoredFreq[i] = ignoredFrequencies[i];
    }
}

// Runs the entire detector: decimating fir-filter, iir-filters,
// power-computation, hit-detection. if interruptsCurrentlyEnabled = true, interrupts
// are running. If interruptsCurrentlyEnabled = false you can pop values from the
// ADC buffer without disabling interrupts. If interruptsCurrentlyEnabled = true,
// do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits that are detected on the frequencies specified during detector_init().
// Your own frequency (based on the switches) is a good choice to ignore.
// Assumption: draining the ADC buffer occurs faster than it can fill.
void detector(bool interruptsCurrentlyEnabled) {

}

// Returns true if a hit was detected.
bool detector_hitDetected() {
    return hitFrequency >= 0;
}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit() {
    return hitFrequency;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit() {
    hitFrequency = -1;
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {
    ignoreHits = flagValue;
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
    // Copy hit counts into given array
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
        hitArray[i] = hitCounts[i];
    }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factor) {
    fudgeFactor = factor;
}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue) {

}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest() {

}