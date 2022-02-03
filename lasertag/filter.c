#include "filter.h"

#define FILTER_IIR_COUNT 10
#define IIR_A_COEFFICIENT_COUNT 10
#define Z_QUEUE_SIZE IIR_A_COEFFICIENT_COUNT
#define OUTPUT_QUEUE_SIZE IIR_A_COEFFICIENT_COUNT
#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE 81
#define QUEUE_INIT_VAL 0

static queue_t zQueue[FILTER_IIR_COUNT];
static queue_t outputQueue[FILTER_IIR_COUNT];
static queue_t xQueue;
static queue_t yQueue;
const static double FIRCoefficients[X_QUEUE_SIZE] = {
    4.3579622275120866e-04, 2.7155425450406482e-04, 6.3039002645022389e-05,
    -1.9349227837935689e-04, -4.9526428865281219e-04, -8.2651441681321381e-04,
    -1.1538970332472540e-03, -1.4254746936265955e-03, -1.5744703111426981e-03,
    -1.5281041447445794e-03, -1.2208092333090719e-03, -6.1008312441271589e-04, 
    3.0761698758506020e-04, 1.4840192333212628e-03, 2.8123077568332064e-03,
    4.1290616416556000e-03, 5.2263464670258821e-03, 5.8739882867061598e-03, 
    5.8504032099208096e-03, 4.9787419333799775e-03, 3.1637974805960069e-03,
    4.2435139609132765e-04, -3.0844289197247210e-03, -7.0632027332701800e-03,
    -1.1078458037608587e-02, -1.4591395057493114e-02, -1.7004337345765962e-02,
    -1.7720830774014484e-02, -1.6213409845727566e-02, -1.2091458677988302e-02,
    -5.1609257765542595e-03, 4.5319860006883522e-03, 1.6679627700682677e-02,
    3.0718365411587255e-02, 4.5861875593064996e-02, 6.1160185621895728e-02,
    7.5579213982547147e-02, 8.8092930943210607e-02, 9.7778502396672365e-02,
    1.0390414346016495e-01
};

// Initialize Z-queues and fill them with zeros
void initZQueues() {
    for (uint32_t i = 0; i < FILTER_IIR_COUNT; i++) {
        // Init each IIR filter
        queue_init(&(zQueue[i], Z_QUEUE_SIZE);

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
        queue_init(&(outputQueue[i], OUTPUT_QUEUE_SIZE);

        for (uint32_t j = 0; j < OUTPUT_QUEUE_SIZE; j++) {
            // Push all zeros to each filter
            queue_overwritePush(&(outputQueue[i]), QUEUE_INIT_VAL);
        }
    }
}

// Initialize xQueue
void initXQueue() {
    queue_init(&xQueue, X_QUEUE_SIZE);

    for (uint32_t j = 0; j < X_QUEUE_SIZE; j++) {
        // Push all zeros to each filter
        queue_overwritePush(&xQueue, QUEUE_INIT_VAL);
    }
}

// Initialize yQueue
void initYQueue() {
    queue_init(&yQueue, Y_QUEUE_SIZE);

    for (uint32_t j = 0; j < Y_QUEUE_SIZE; j++) {
        // Push all zeros to each filter
        queue_overwritePush(&yQueue, QUEUE_INIT_VAL);
    }
}

// Must call this prior to using any filter functions.
void filter_init() {
    initZQueues();
    initOutputQueues();
    initXQueue();
    initYQueue();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x) {
    queue_push(&xQueue, x);
}

// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
void filter_fillQueue(queue_t *q, double fillValue) {
    
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter() {
    double output = 0;

    for (uin32_t i = 0; i < X_QUEUE_SIZE; i++) {
        output += FIRCoefficients[i] * queue_readElementAt(&xQueue, X_QUEUE_SIZE - 1 - i);
    }

    return output;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber) {

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

}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber) {

}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]) {

}

// Using the previously-computed power values that are current stored in
// currentPowerValue[] array, Copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue) {

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
const double *filter_getFirCoefficientArray() {

}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() {

}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {

}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() {

}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {

}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() {

}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() {

}

// Returns the decimation value.
uint16_t filter_getDecimationValue() {

}

// Returns the address of xQueue.
queue_t *filter_getXQueue() {

}

// Returns the address of yQueue.
queue_t *filter_getYQueue() {

}

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {

}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {

}