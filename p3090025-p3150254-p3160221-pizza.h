#include <stdio.h>

// Used by customer threads to order pizzas.
void *order(void *args);

// Generate random number given a minimum and a maximum number.
int generateRandomNumber(int min, int max, unsigned int *seed);

// Check that provided arguments are arithmetic values.
void checkValidArguments(char args[]);

// Lock mutex and perform checks.
void mutexLock(pthread_mutex_t* mutex);

// Unlock mutex and perform checks.
void mutexUnlock(pthread_mutex_t* mutex);

// Signal threads waiting and perform checks.
void condBroadcast(pthread_cond_t* cond);

// Wait and perform checks.
void condWait(pthread_cond_t* cond, pthread_mutex_t* mutex);

void destroyMutex(pthread_mutex_t *mutex);

// Number of available tels.
const int TELS = 3;
// Number of available cooks.
const int COOKS = 2;
// Number of available ovens;
const int OVENS = 10;
// Number of avaiable deliverers.
const int DELIVERERS = 7;

// Minimum and maximum time to order.
const int T_ORDER_LOW = 1;
const int T_ORDER_HIGH = 5;

// Minimum and maximum time to process payment.
const int T_PAYMENT_LOW = 1;
const int T_PAYMENT_HIGH = 2;

// Minimum and maximum time to deliver order.
const int T_DELIVER_LOW = 5;
const int T_DELIVER_HIGH = 15;

// Time to prepapare a pizza.
const int T_PREP = 1;
// Time to bake a pizza.
const int T_BAKE = 10;
// Time to pack a pizza.
const int T_PACK = 2;

// Minimum and maximum number of pizzas in order.
const int N_ORDER_LOW = 1;
const int N_ORDER_HIGH = 5;

// Cost of single pizza.
const double COST = 10;

// Probability the payment fails.
const double P = 0.05;
