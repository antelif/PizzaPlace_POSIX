#include <stdio.h>

// Used by customer threads to order pizzas.
void *order(void *args);

// Generate random number given a minimum and a maximum number.
int generateRandomNumber(int min, int max, unsigned int *seed);

// Number of available tels.
const int TELS = 2;
// Number of available cooks.
const int COOKS = 2;
// Number of available ovens;
const int OVENS = 2;
// Number of avaiable deliverers.
const int DELIVERERS = 3;

// Minimum and maximum time to order.
const int T_ORDER_LOW = 1;
const int T_ORDER_HIGH = 5;

// Minimum and maximum time to process payment.
const int T_PAYMENT_LOW = 1;
const int T_PAYMENT_HIGH = 10;

// Minimum and maximum time to deliver order.
const int T_DELIVER_LOW = 1;
const int T_DELIVER_HIGH = 5;

// Time to prepapare a pizza.
const int T_PREP = 2;
// Time to bake a pizza.
const int T_BAKE = 3;
// Time to pack a pizza.
const int T_PACK = 5;

// Minimum and maximum number of pizzas in order.
const int N_ORDER_LOW = 1;
const int N_ORDER_HIGH = OVENS;

// Cost of single pizza.
const double COST = 7.5;

// Probability the payment fails.
const double P = 0.15;
