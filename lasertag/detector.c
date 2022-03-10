#include "detector.h"
#include "filter.h"
#include "isr.h"
#include "interrupts.h"
#include "stdio.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"

#define MAX_ADCVALUE 4095
#define ADC_RANGE 2 
#define NO_HIT -1
#define MEDIAN_INDEX 5

static detector_hitCount_t hitCounts[FILTER_FREQUENCY_COUNT];
static bool ignoredFreq[FILTER_FREQUENCY_COUNT];
static double computedPower[FILTER_FREQUENCY_COUNT];
static int16_t filterIndexArray[FILTER_FREQUENCY_COUNT];
static uint32_t fudgeFactor = 1000;
static int16_t hitFrequency = NO_HIT;
static bool ignoreHits = false;
static bool hitDetected = false;

static double testArray[FILTER_FREQUENCY_COUNT] = { 5,6,2,1,8,7,4,9,3,0 };

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

// Use a selection sort algorithm to sort the power indices
void sortPower() {
    double key;
    int32_t indexKey, j;
    for (uint16_t i = 1; i < FILTER_FREQUENCY_COUNT; i++) {
        key = computedPower[i];
        indexKey = filterIndexArray[i];
        j = i - 1;
 
        /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
        while (j >= 0 && computedPower[j] > key) {
            computedPower[j + 1] = computedPower[j];
            filterIndexArray[j + 1] = filterIndexArray[j];
            j = j - 1;
        }
        filterIndexArray[j + 1] = indexKey;
        computedPower[j + 1] = key;
    }
}

// Detect which frequency hit the player
void detectHit() {
    sortPower();
    double threshold = computedPower[MEDIAN_INDEX] * fudgeFactor;
    int16_t hitFreq = computedPower[FILTER_FREQUENCY_COUNT - 1] > threshold
            ? filterIndexArray[FILTER_FREQUENCY_COUNT - 1]
            : NO_HIT;

    hitFrequency = ignoredFreq[hitFreq] ? NO_HIT : hitFreq;
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
    uint16_t elementCount = isr_adcBufferElementCount();
    uint16_t filterCallCount = 0;

    // repeats steps for elementCount size
    for (uint16_t i = 0; i < elementCount; i++) {
        isr_AdcValue_t rawAdcValue;

        // disables interrupts if interrupts are enabled
        // pops value from adcBuffer, then enables interrupts
        if (interruptsCurrentlyEnabled) {
            interrupts_disableArmInts();
            rawAdcValue = isr_removeDataFromAdcBuffer();
            interrupts_enableArmInts();
        }
        else { 
            rawAdcValue = isr_removeDataFromAdcBuffer();
        }

        double scaledAdcValue = detector_getScaledAdcValue(rawAdcValue);
        filter_addNewInput(scaledAdcValue);
        filterCallCount++;

        // filter addNewInput count has been called 10 times
        if (filterCallCount == FILTER_FREQUENCY_COUNT) {
            filter_firFilter();

            //for each filter run the filters and power computation
            for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++){
                filter_iirFilter(i);
                computedPower[i] = filter_computePower(i, false, false);
                filterIndexArray[i] = i;
            }
            // If not in lockout, detect if a hit has taken place
            if (!lockoutTimer_running()){
                detectHit();

                // Frequency is valid, start timers and register hit
                if (hitFrequency != NO_HIT && !ignoreHits) {
                    lockoutTimer_start();
                    hitLedTimer_start();
                    hitCounts[hitFrequency]++;
                    hitDetected = true;
                }
            }

            filterCallCount = 0;
        }
    }
}

// Returns true if a hit was detected.
bool detector_hitDetected() {
    return hitDetected;
}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit() {
    return hitFrequency;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit() {
    hitFrequency = NO_HIT;
    hitDetected = false;
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
    return (((double)(adcValue / MAX_ADCVALUE) * ADC_RANGE) - 1);
}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest() {
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
        computedPower[i] = testArray[i];
        filterIndexArray[i] = i;
        printf("Starting power: %f\n", computedPower[i]);
    }
    
    detectHit();
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        printf("Sorted Power: %f\n", computedPower[i]);
    }
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
        printf("Index Ranked: %d\n", filterIndexArray[i]);
    }
}