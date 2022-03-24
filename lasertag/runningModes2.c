/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "interrupts.h"
#include "runningModes.h"
#include "hitLedTimer.h"
#include "sound.h"
#include "autoReloadTimer.h"
#include "invincibilityTimer.h"
#include <stdio.h>

#define ONE_SECOND_DELAY 1000

/*
This file (runningModes2.c) is separated from runningModes.c so that
check_and_zip.py does not include provided code for grading. Code for
submission can be added to this file and will be graded. The code in
runningModes.c can give you some ideas about how to implement other
modes here.
*/

// Custom running mode for 2 teams
void runningModes_twoTeams() {
  uint16_t hitCount = 0;
  // More initialization...

  // Implement game loop...

  interrupts_disableArmInts(); // Done with game loop, disable the interrupts.
  hitLedTimer_turnLedOff();    // Save power :-)
  runningModes_printRunTimeStatistics(); // Print the run-time statistics to the
                                         // TFT.
  printf("Two-team mode terminated after detecting %d shots.\n", hitCount);
}

// Last-man standing game mode
void runningModes_lastPersonStanding() {
  runningModes_initAll();
  // Init the ignored-frequencies so no frequencies are ignored.
  bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
  for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++)
    ignoredFrequencies[i] = false;
  ignoredFrequencies[runningModes_getFrequencySetting()] = true;
  detector_init(ignoredFrequencies);
  uint16_t hitCount = 0;
  detectorInvocationCount = 0; // Keep track of detector invocations.
  trigger_enable();         // Makes the trigger state machine responsive to the
                            // trigger.
  interrupts_initAll(true); // Inits all interrupts but does not enable them.
  interrupts_enableTimerGlobalInts(); // Allows the timer to generate
                                      // interrupts.
  interrupts_startArmPrivateTimer();  // Start the private ARM timer running.
  uint16_t histogramSystemTicks =
      0; // Only update the histogram display every so many ticks.
  intervalTimer_reset(
      ISR_CUMULATIVE_TIMER); // Used to measure ISR execution time.
  intervalTimer_reset(
      TOTAL_RUNTIME_TIMER); // Used to measure total program execution time.
  intervalTimer_reset(
      MAIN_CUMULATIVE_TIMER); // Used to measure main-loop execution time.
  intervalTimer_start(
      TOTAL_RUNTIME_TIMER);   // Start measuring total execution time.
  interrupts_enableArmInts(); // The ARM will start seeing interrupts after
                              // this.
  lockoutTimer_start(); // Ignore erroneous hits at startup (when all power
                        // values are essentially 0).

  trigger_setRemainingShotCount(LAST_STANDING_AMMO);

  while (hitCount >= 15) &&
         hitCount < MAX_HIT_COUNT) { // Run until you detect btn3 pressed.
    transmitter_setFrequencyNumber(
        runningModes_getFrequencySetting());    // Read the switches and switch
                                                // frequency as required.
    intervalTimer_start(MAIN_CUMULATIVE_TIMER); // Measure run-time when you are
                                                // doing something.
    histogramSystemTicks++; // Keep track of ticks so you know when to update
                            // the histogram.
    // Run filters, compute power, run hit-detection.
    detectorInvocationCount++;              // Used for run-time statistics.
    detector(INTERRUPTS_CURRENTLY_ENABLED); // Interrupts are currently enabled.
    if (trigger_getRemainingShotCount() == 0) autoReloadTimer_start();

    if (detector_hitDetected()) {           // Hit detected
      hitCount++;                           // increment the hit count.
      if (hitCount % 5 === 0) {
        sound_playSound(sound_loseLife_e);
        invincibilityTimer_start();
      }
      else sound_playSound(sound_hit_e);
      detector_clearHit();                  // Clear the hit.
      detector_hitCount_t
          hitCounts[DETECTOR_HIT_ARRAY_SIZE]; // Store the hit-counts here.
      detector_getHitCounts(hitCounts);       // Get the current hit counts.
      histogram_plotUserHits(hitCounts);      // Plot the hit counts on the TFT.
    }

    intervalTimer_stop(
        MAIN_CUMULATIVE_TIMER); // All done with actual processing.
  }

  sound_playSound(sound_gameOver_e);

  // Return to base sound
  while (1) {
    sound_playSound(sound_returnToBase_e);
    // Wait until the sound has finished playing
    while (sound_isBusy());
    utils_msDelay(ONE_SECOND_DELAY);
  }

  interrupts_disableArmInts(); // Done with loop, disable the interrupts.
  hitLedTimer_turnLedOff();    // Save power :-)
  runningModes_printRunTimeStatistics(); // Print the run-time statistics to the
                                         // TFT.
  printf("Shooter mode terminated after detecting %d shots.\n", hitCount);
}
