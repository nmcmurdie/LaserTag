#include "filter.h"
#include <stdio.h>

#define FILTER_IIR_COUNT 10
#define IIR_A_COEFFICIENT_COUNT 10
#define Z_QUEUE_SIZE IIR_A_COEFFICIENT_COUNT
#define OUTPUT_QUEUE_SIZE 2000
#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE IIR_B_COEFFICIENT_COUNT
#define QUEUE_INIT_VAL 0
#define FIR_COEFFICIENT_COUNT 81
#define FILTER_FREQUENCY_COUNT 10
#define IIR_A_COEFFICIENT_COUNT 10
#define IIR_B_COEFFICIENT_COUNT 11
#define QUEUE_OLDEST_ELEM 0
#define OUTPUT_QUEUE_SIZE 2000
#define FILTER_FIR_DECIMATION_FACTOR 10
#define X_QUEUE_NAME "X-Queue"
#define Y_QUEUE_NAME "Y-Queue"
#define Z_QUEUE_NAME "Z-Queue"
#define OUTPUT_QUEUE_NAME "Output-Queue"

static queue_t zQueue[FILTER_IIR_COUNT];
static queue_t outputQueue[FILTER_IIR_COUNT];
static queue_t xQueue;
static queue_t yQueue;
static double prevPower[FILTER_FREQUENCY_COUNT];
static double oldestValue[FILTER_FREQUENCY_COUNT];
static double currentPowerValue[FILTER_FREQUENCY_COUNT];
const static double FIRCoefficients[FIR_COEFFICIENT_COUNT] = {
    4.3579622275120866e-04,  2.7155425450406482e-04,  6.3039002645022389e-05,
    -1.9349227837935689e-04, -4.9526428865281219e-04, -8.2651441681321381e-04,
    -1.1538970332472540e-03, -1.4254746936265955e-03, -1.5744703111426981e-03,
    -1.5281041447445794e-03, -1.2208092333090719e-03, -6.1008312441271589e-04,
    3.0761698758506020e-04,  1.4840192333212628e-03,  2.8123077568332064e-03,
    4.1290616416556000e-03,  5.2263464670258821e-03,  5.8739882867061598e-03,
    5.8504032099208096e-03,  4.9787419333799775e-03,  3.1637974805960069e-03,
    4.2435139609132765e-04,  -3.0844289197247210e-03, -7.0632027332701800e-03,
    -1.1078458037608587e-02, -1.4591395057493114e-02, -1.7004337345765962e-02,
    -1.7720830774014484e-02, -1.6213409845727566e-02, -1.2091458677988302e-02,
    -5.1609257765542595e-03, 4.5319860006883522e-03,  1.6679627700682677e-02,
    3.0718365411587255e-02,  4.5861875593064996e-02,  6.1160185621895728e-02,
    7.5579213982547147e-02,  8.8092930943210607e-02,  9.7778502396672365e-02,
    1.0390414346016495e-01,  1.0600000000000000e-01,  1.0390414346016495e-01,
    9.7778502396672365e-02,  8.8092930943210607e-02,  7.5579213982547147e-02,
    6.1160185621895728e-02,  4.5861875593064996e-02,  3.0718365411587255e-02,
    1.6679627700682677e-02,  4.5319860006883522e-03,  -5.1609257765542595e-03,
    -1.2091458677988302e-02, -1.6213409845727566e-02, -1.7720830774014484e-02,
    -1.7004337345765962e-02, -1.4591395057493114e-02, -1.1078458037608587e-02,
    -7.0632027332701800e-03, -3.0844289197247210e-03, 4.2435139609132765e-04,
    3.1637974805960069e-03,  4.9787419333799775e-03,  5.8504032099208096e-03,
    5.8739882867061598e-03,  5.2263464670258821e-03,  4.1290616416556000e-03,
    2.8123077568332064e-03,  1.4840192333212628e-03,  3.0761698758506020e-04,
    -6.1008312441271589e-04, -1.2208092333090719e-03, -1.5281041447445794e-03,
    -1.5744703111426981e-03, -1.4254746936265955e-03, -1.1538970332472540e-03,
    -8.2651441681321381e-04, -4.9526428865281219e-04, -1.9349227837935689e-04,
    6.3039002645022389e-05,  2.7155425450406482e-04,  4.3579622275120866e-04};
const static double iirACoefficientConstants
    [FILTER_FREQUENCY_COUNT][IIR_A_COEFFICIENT_COUNT] = {
        {-5.9637727070164015e+00, 1.9125339333078248e+01,
         -4.0341474540744173e+01, 6.1537466875368821e+01,
         -7.0019717951472188e+01, 6.0298814235238872e+01,
         -3.8733792862566290e+01, 1.7993533279581058e+01,
         -5.4979061224867651e+00, 9.0332828533799547e-01},
        {-4.6377947119071505e+00, 1.3502215749461598e+01,
         -2.6155952405269829e+01, 3.8589668330738476e+01,
         -4.3038990303252795e+01, 3.7812927599537275e+01,
         -2.5113598088113893e+01, 1.2703182701888142e+01,
         -4.2755083391143689e+00, 9.0332828533800569e-01},
        {-3.0591317915750973e+00, 8.6417489609637634e+00,
         -1.4278790253808870e+01, 2.1302268283304350e+01,
         -2.2193853972079282e+01, 2.0873499791105495e+01,
         -1.3709764520609433e+01, 8.1303553577931904e+00,
         -2.8201643879900606e+00, 9.0332828533800258e-01},
        {-1.4071749185996785e+00, 5.6904141470697605e+00,
         -5.7374718273676475e+00, 1.1958028362868930e+01,
         -8.5435280598354932e+00, 1.1717345583835993e+01,
         -5.5088290876998860e+00, 5.3536787286077807e+00,
         -1.2972519209655651e+00, 9.0332828533800258e-01},
        {8.2010906117760696e-01, 5.1673756579268639e+00, 3.2580350909221076e+00,
         1.0392903763919204e+01, 4.8101776408669306e+00, 1.0183724507092521e+01,
         3.1282000712126905e+00, 4.8615933365572053e+00, 7.5604535083145297e-01,
         9.0332828533800136e-01},
        {2.7080869856154512e+00, 7.8319071217995688e+00, 1.2201607990980744e+01,
         1.8651500443681620e+01, 1.8758157568004549e+01, 1.8276088095999022e+01,
         1.1715361303018897e+01, 7.3684394621253499e+00, 2.4965418284511904e+00,
         9.0332828533800436e-01},
        {4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01,
         4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01,
         2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00,
         9.0332828533799958e-01},
        {6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071684e+01,
         6.5958045321253451e+01, 7.5230437667866596e+01, 6.4630411355739852e+01,
         4.1261591079244127e+01, 1.8936128791950534e+01, 5.6881982915180291e+00,
         9.0332828533799803e-01},
        {7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01,
         9.8258255839887312e+01, 1.1359460153696298e+02, 9.6280452143026082e+01,
         5.9124742025776392e+01, 2.5268527576524203e+01, 6.8305064480743081e+00,
         9.0332828533799969e-01},
        {8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037053e+01,
         1.3928510844056814e+02, 1.6305115418161620e+02, 1.3648147221895786e+02,
         8.0686288623299745e+01, 3.2276361903872115e+01, 7.9045143816244696e+00,
         9.0332828533799636e-01}};
const static double
    iirBCoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_B_COEFFICIENT_COUNT] =
        {{9.0928661148194738e-10, 0.0000000000000000e+00,
          -4.5464330574097372e-09, 0.0000000000000000e+00,
          9.0928661148194745e-09, 0.0000000000000000e+00,
          -9.0928661148194745e-09, 0.0000000000000000e+00,
          4.5464330574097372e-09, 0.0000000000000000e+00,
          -9.0928661148194738e-10},
         {9.0928661148175165e-10, 0.0000000000000000e+00,
          -4.5464330574087587e-09, 0.0000000000000000e+00,
          9.0928661148175174e-09, 0.0000000000000000e+00,
          -9.0928661148175174e-09, 0.0000000000000000e+00,
          4.5464330574087587e-09, 0.0000000000000000e+00,
          -9.0928661148175165e-10},
         {9.0928661148192040e-10, 0.0000000000000000e+00,
          -4.5464330574096024e-09, 0.0000000000000000e+00,
          9.0928661148192048e-09, 0.0000000000000000e+00,
          -9.0928661148192048e-09, 0.0000000000000000e+00,
          4.5464330574096024e-09, 0.0000000000000000e+00,
          -9.0928661148192040e-10},
         {9.0928661148180532e-10, 0.0000000000000000e+00,
          -4.5464330574090267e-09, 0.0000000000000000e+00,
          9.0928661148180534e-09, 0.0000000000000000e+00,
          -9.0928661148180534e-09, 0.0000000000000000e+00,
          4.5464330574090267e-09, 0.0000000000000000e+00,
          -9.0928661148180532e-10},
         {9.0928661148211809e-10, 0.0000000000000000e+00,
          -4.5464330574105901e-09, 0.0000000000000000e+00,
          9.0928661148211801e-09, 0.0000000000000000e+00,
          -9.0928661148211801e-09, 0.0000000000000000e+00,
          4.5464330574105901e-09, 0.0000000000000000e+00,
          -9.0928661148211809e-10},
         {9.0928661148179839e-10, 0.0000000000000000e+00,
          -4.5464330574089919e-09, 0.0000000000000000e+00,
          9.0928661148179839e-09, 0.0000000000000000e+00,
          -9.0928661148179839e-09, 0.0000000000000000e+00,
          4.5464330574089919e-09, 0.0000000000000000e+00,
          -9.0928661148179839e-10},
         {9.0928661148193684e-10, 0.0000000000000000e+00,
          -4.5464330574096843e-09, 0.0000000000000000e+00,
          9.0928661148193686e-09, 0.0000000000000000e+00,
          -9.0928661148193686e-09, 0.0000000000000000e+00,
          4.5464330574096843e-09, 0.0000000000000000e+00,
          -9.0928661148193684e-10},
         {9.0928661148195069e-10, 0.0000000000000000e+00,
          -4.5464330574097538e-09, 0.0000000000000000e+00,
          9.0928661148195076e-09, 0.0000000000000000e+00,
          -9.0928661148195076e-09, 0.0000000000000000e+00,
          4.5464330574097538e-09, 0.0000000000000000e+00,
          -9.0928661148195069e-10},
         {9.0928661148190954e-10, 0.0000000000000000e+00,
          -4.5464330574095478e-09, 0.0000000000000000e+00,
          9.0928661148190956e-09, 0.0000000000000000e+00,
          -9.0928661148190956e-09, 0.0000000000000000e+00,
          4.5464330574095478e-09, 0.0000000000000000e+00,
          -9.0928661148190954e-10},
         {9.0928661148206091e-10, 0.0000000000000000e+00,
          -4.5464330574103047e-09, 0.0000000000000000e+00,
          9.0928661148206094e-09, 0.0000000000000000e+00,
          -9.0928661148206094e-09, 0.0000000000000000e+00,
          4.5464330574103047e-09, 0.0000000000000000e+00,
          -9.0928661148206091e-10}};

// Initialize Z-queues and fill them with zeros
void initZQueues() {
  for (uint32_t i = 0; i < FILTER_IIR_COUNT; i++) {
    // Init each IIR filter
    queue_init(&(zQueue[i]), Z_QUEUE_SIZE, Z_QUEUE_NAME);

    for (uint32_t j = 0; j < Z_QUEUE_SIZE; j++) {
      // Push all zeros to each filter
      queue_overwritePush(&(zQueue[i]), QUEUE_INIT_VAL);
    }
  }
}

// Initialize output queues and fill them with zeros
void initOutputQueues() {
  for (uint32_t i = 0; i < FILTER_IIR_COUNT; i++) {
    // Init each IIR filter
    queue_init(&(outputQueue[i]), OUTPUT_QUEUE_SIZE, OUTPUT_QUEUE_NAME);

    for (uint32_t j = 0; j < OUTPUT_QUEUE_SIZE; j++) {
      // Push all zeros to each filter
      queue_overwritePush(&(outputQueue[i]), QUEUE_INIT_VAL);
    }
  }
}

// Initialize xQueue
void initXQueue() {
  queue_init(&xQueue, X_QUEUE_SIZE, X_QUEUE_NAME);

  for (uint32_t j = 0; j < X_QUEUE_SIZE; j++) {
    // Push all zeros to each filter
    queue_overwritePush(&xQueue, QUEUE_INIT_VAL);
  }
}

// Initialize yQueue
void initYQueue() {
  queue_init(&yQueue, Y_QUEUE_SIZE, Y_QUEUE_NAME);

  for (uint32_t j = 0; j < Y_QUEUE_SIZE; j++) {
    // Push all zeros to each filter
    queue_overwritePush(&yQueue, QUEUE_INIT_VAL);
  }
}

// Initialize odlestValues
void initValues() {
  //queue_init(&yQueue, Y_QUEUE_SIZE, Y_QUEUE_NAME);

  for (uint32_t j = 0; j < FILTER_FREQUENCY_COUNT; j++) {
    // Push all zeros to each filter
    //queue_overwritePush(&yQueue, QUEUE_INIT_VAL);
    oldestValue[j]=0;
    prevPower[j]=0;
    currentPowerValue[j]=0;
    
  }
}
// Must call this prior to using any filter functions.
void filter_init() {
  initZQueues();
  initOutputQueues();
  initXQueue();
  initYQueue();
  initValues();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x) { queue_overwritePush(&xQueue, x); }

// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
void filter_fillQueue(queue_t *q, double fillValue) {
  queue_size_t size = queue_size(q);

  for (uint32_t i = 0; i < size; i++) {
    // Overwrite contents of queue with the fill value
    queue_overwritePush(q, fillValue);
  }
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter() {
  double output = 0.0;

  for (uint32_t i = 0; i < X_QUEUE_SIZE; i++) {
    // Process the queue using the FIR filter coefficients
    output +=
        FIRCoefficients[i] * queue_readElementAt(&xQueue, X_QUEUE_SIZE - 1 - i);
  }

  queue_overwritePush(&yQueue, output);
  return output;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber) {
  double output = 0.0;

  for (uint32_t i = 0; i < IIR_B_COEFFICIENT_COUNT; i++) {
    // Process the queue using the IIR B filter coefficients
    output += iirBCoefficientConstants[filterNumber][i] *
              queue_readElementAt(&yQueue, IIR_B_COEFFICIENT_COUNT - 1 - i);
  }

  for (uint32_t i = 0; i < IIR_A_COEFFICIENT_COUNT; i++) {
    // Process the queue using the IIR A filter coefficients
    output -= iirACoefficientConstants[filterNumber][i] *
              queue_readElementAt(&(zQueue[filterNumber]),
                                  IIR_A_COEFFICIENT_COUNT - 1 - i);
  }

  queue_overwritePush(&(zQueue[filterNumber]), output);
  queue_overwritePush(&(outputQueue[filterNumber]), output);
  return output;
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint) {
  double power = 0.0;

  if (forceComputeFromScratch) {
    // Compute the power from output queues from scratch
    for (uint32_t i = 0; i < OUTPUT_QUEUE_SIZE; i++) {
      // Sum each element in the output queue squared
      double queueElem = queue_readElementAt(&(outputQueue[filterNumber]), i);
      power += queueElem * queueElem;
    }
  } else {
    // Not computing from scratch, remove oldest value and add newest value
    // squared
    double newestValue = queue_readElementAt(&(outputQueue[filterNumber]),
                                             OUTPUT_QUEUE_SIZE - 1);
                                             
    power = prevPower[filterNumber] -
            (oldestValue[filterNumber] * oldestValue[filterNumber]) +
            (newestValue * newestValue);
            
  }

  prevPower[filterNumber] = power;
  oldestValue[filterNumber] =
      queue_readElementAt(&(outputQueue[filterNumber]), QUEUE_OLDEST_ELEM);
      //printf("power: %8.4e\n",oldestValue[filterNumber]);
  currentPowerValue[filterNumber] = power;
  //printf("POWER VALUE: %f\n",power);
  return power;
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber) {
  return currentPowerValue[filterNumber];
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]) {
  for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    // Copy values from our power value array into the provided array
    powerValues[i] = currentPowerValue[i];
    //printf("POWER VALUE: %f\n",powerValues[i]);
  }
}

// Using the previously-computed power values that are current stored in
// currentPowerValue[] array, Copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue) {
  double maxValue = 0.0;
  for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    // Iterate over each power value
    if (currentPowerValue[i] > maxValue) {
      // If the power is at its max, keep the value and index
      *indexOfMaxValue = i;
      maxValue = currentPowerValue[i];
    }
  }

  for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    // Copy over power values and normalize
    normalizedArray[i] = currentPowerValue[i] / maxValue;
  }
}

/*********************************************************************************************************
********************************** Verification-assisting functions.
**************************************
********* Test functions access the internal data structures of the filter.c via
*these functions. ********
*********************** These functions are not used by the main filter
*functions. ***********************
**********************************************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray() { return FIRCoefficients; }

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() { return FIR_COEFFICIENT_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
  return iirACoefficientConstants[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() { return IIR_A_COEFFICIENT_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
  return iirBCoefficientConstants[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() { return IIR_B_COEFFICIENT_COUNT; }

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() { return Y_QUEUE_SIZE; }

// Returns the decimation value.
uint16_t filter_getDecimationValue() { return FILTER_FIR_DECIMATION_FACTOR; }

// Returns the address of xQueue.
queue_t *filter_getXQueue() { return &xQueue; }

// Returns the address of yQueue.
queue_t *filter_getYQueue() { return &yQueue; }

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {
  return &(zQueue[filterNumber]);
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
  return &(outputQueue[filterNumber]);
}
