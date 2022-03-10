#include "isr.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include "transmitter.h"
#include "trigger.h"
#include "interrupts.h"

#define ADC_BUFFER_SIZE 100000
#define ADC_BUFFER_EMPTY 0

// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by detector().
// adcBuffer_t is similar to a queue.
typedef struct {
	uint32_t indexIn;   // New values go here.
	uint32_t indexOut;  // Pull old values from here.
	uint32_t elementCount; // Number of elements in the buffer.
	uint32_t data[ADC_BUFFER_SIZE];  // Values are stored here.
} adcBuffer_t;

// This is the instantiation of adcBuffer.
volatile static adcBuffer_t adcBuffer;

// Init ADC Buffer
void adcBufferInit() {
  // Initialize each element to 0
  for (uint32_t i = 0; i < adcBuffer.elementCount; i++) {
    adcBuffer.data[i] = 0;
  }
  adcBuffer.indexIn = 0;
  adcBuffer.indexOut = 0;
  adcBuffer.elementCount = 0;
}

// Performs inits for anything in isr.c
void isr_init() {
  adcBufferInit();
  transmitter_init();
  lockoutTimer_init();
  hitLedTimer_init();
  trigger_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
  transmitter_tick();
  lockoutTimer_tick();
  hitLedTimer_tick();
  trigger_tick();
  isr_addDataToAdcBuffer(interrupts_getAdcData());
}

// This adds data to the ADC queue. Data are removed from this queue and used by
// the detector.
void isr_addDataToAdcBuffer(uint32_t adcData) {
  adcBuffer.data[adcBuffer.indexIn] = adcData;
  adcBuffer.indexIn = (adcBuffer.indexIn + 1) % ADC_BUFFER_SIZE;

  // Only increment element count if buffer isn't full
  if (adcBuffer.elementCount != ADC_BUFFER_SIZE) adcBuffer.elementCount++;
}

// This removes a value from the ADC buffer.
uint32_t isr_removeDataFromAdcBuffer() {
  // If the buffer is empty, simply return 0
  if (adcBuffer.elementCount == ADC_BUFFER_EMPTY) return ADC_BUFFER_EMPTY;

  uint32_t pop = adcBuffer.data[adcBuffer.indexOut];
  adcBuffer.indexOut = (adcBuffer.indexOut + 1) % ADC_BUFFER_SIZE;
  adcBuffer.elementCount--;

  return pop;
}

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount() {
  return adcBuffer.elementCount;
}