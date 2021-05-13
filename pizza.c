#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "pizza.h"

// Number threads to be created for each category.
int CUSTOMERS;

// Availability of employees.
int availableTels = TELS;
int availableCooks = COOKS;
int availableOvens = OVENS;
int availablePacketers = 1;
int availableDeliveres = DELIVERERS;

// Total revenue at the end of the day.
int revenue = 0;

int unsuccessfulOrders = 0;

// Time statistics.
int maxWaitTime = 0;
int totalWaitTime = 0;

int maxOrderTime = 0;
int totalOrderTime = 0;

int maxCoolTime = 0;
int totalCoolTime = 0;

// Seed to create random numbers
unsigned int seed;

int printReady = 0;

// Mutexes.
pthread_mutex_t printMutex;

pthread_mutex_t waitTimeMutex;
pthread_mutex_t orderTimeMutex;
pthread_mutex_t coolTimeMutex;

pthread_mutex_t ordersFailMutex;
pthread_mutex_t revenueMutex;

pthread_mutex_t telsMutex;
pthread_mutex_t cooksMutex;
pthread_mutex_t ovensMutex;
pthread_mutex_t packeterMutex;
pthread_mutex_t deliverersMutex;

// Conditions.
pthread_cond_t printCond;

pthread_cond_t telsCond;
pthread_cond_t cooksCond;
pthread_cond_t ovensCond;
pthread_cond_t packeterCond;
pthread_cond_t delivererCond;

// Lock mutex and perform checks.
void mutexLock(pthread_mutex_t *args){
  int rc = pthread_mutex_lock(args);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
}

// Unlock mutex and perform checks.
void mutexUnlock(pthread_mutex_t *args){
  int rc = pthread_mutex_unlock(args);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }
}

// Signal threads waiting and perform checks.
void condBroadcast(pthread_cond_t* cond){
  int rc = pthread_cond_broadcast(cond);
  if (rc != 0) {
    perror("ERROR: Cond brodcast failed.\n");
    pthread_exit(&rc);
  }
}

// Wait and perform checks.
void condWait(pthread_cond_t* cond, pthread_mutex_t* mutex){
  int rc = pthread_cond_wait(cond, mutex);
  if (rc != 0) {
    perror("ERROR: Cond wait failed.\n");
    pthread_exit(&rc);
  }
}

// Generate random number given a minimum and a maximum number.
int generateRandomNumber(int min, int max, unsigned int * seed){
  int randomNumber = rand_r(seed);
  // printf("Random number %d\n",randomNumber);
  randomNumber = (randomNumber%(max-min+1))+min;
  // printf("Random number after modulo %d\n", randomNumber);
  return randomNumber;
  // return rand_r(seed)%(max-min)+min;
}

// Used by customer threads to order pizzas.
void *order(void *args){
  // Cast void to int to get id.
  int *id = (int*)args;

  struct timespec startWait, endWait, startCool, endPacket,endOrder;

  // Integer representation of above timespec structs.
  int startWaitTime, endWaitTime, startCoolTime;

  // Total order time for this order.
  int orderTime;
  // Total wait time for this order.
  int waitTime;
  // Total cool time for this order.
  int coolTime;
  // Total delivery time for this order.
  int deliveryTime;

  // 1. ORDERING ---------------------------------------------------------------
  startWaitTime = clock_gettime(CLOCK_REALTIME, &startWait);

  // Wait fot available TEL.
  mutexLock(&telsMutex);

  while(availableTels == 0){
    condWait(&telsCond, &telsMutex);
  }
  availableTels--;

  mutexUnlock(&telsMutex);

  // Calculate wait time for this order.
  endWaitTime = clock_gettime(CLOCK_REALTIME, &endWait);
  waitTime = endWait.tv_sec - startWait.tv_sec;

  // Add this waitTime to global totalWaitTime.
  mutexLock(&waitTimeMutex);

  totalWaitTime += waitTime;

  mutexUnlock(&waitTimeMutex);

  // Customer chooses a number of pizzas.
  int numPizzas = generateRandomNumber(N_ORDER_LOW, N_ORDER_HIGH, &seed);

  //2. PAYMENT -----------------------------------------------------------------
  // Calculate cost.
  int totalPayment = COST * numPizzas;

  // Probability the order will fail. If pSuccess < P then the order fails.
  double pSuccess = (double)rand_r(&seed)/(double)RAND_MAX;

  sleep(generateRandomNumber(T_PAYMENT_LOW, T_PAYMENT_HIGH, &seed));

  if(pSuccess < P){ // Fail.

    // Print fail meassage.
    mutexLock(&printMutex);
    while(printReady != 0){
      condWait(&printCond, &printMutex);
    }
    printReady = 1;

    mutexUnlock(&printMutex);

    printf("Order %d: Payment failed. Canceling...\n",*id);

    mutexLock(&printMutex);

    printReady = 0;
    condBroadcast(&printCond);

    mutexUnlock(&printMutex);

    // Increase counter of failed orders.
    mutexLock(&telsMutex);

    unsuccessfulOrders++;
    availableTels++;

    pthread_cond_broadcast(&telsCond);

    mutexUnlock(&telsMutex);

    pthread_exit(0);
  }else{ // Success.
    // Print success message.
    mutexLock(&printMutex);

    while(printReady != 0){
      condWait(&printCond, &printMutex);
    }
    printReady = 1;

    mutexUnlock(&printMutex);

    printf("Order %d: Payment OK.\n", *id);

    mutexLock(&printMutex);
    printReady = 0;
    condBroadcast(&printCond);

    mutexUnlock(&printMutex);

    // Increase total revenue.
    mutexLock(&revenueMutex);
    revenue += totalPayment;
    mutexUnlock(&revenueMutex);
  }

  // Free tels and signal other threads that wait for TELS.
  mutexLock(&telsMutex);

  availableTels++;
  condBroadcast(&telsCond);

  mutexUnlock(&telsMutex);

  // 3. PREPARE ----------------------------------------------------------------

  // Wait for available cook.
  mutexLock(&cooksMutex);

  while(availableCooks == 0){
    condWait(&cooksCond, &cooksMutex);
  }

  availableCooks--;

  mutexUnlock(&cooksMutex);

  // Prepare pizzas.
  sleep(T_PREP * numPizzas);

  // 4. BAKE -------------------------------------------------------------------

  // Wait for available ovens.
  mutexLock(&ovensMutex);

  while(availableOvens < numPizzas){
    pthread_cond_wait(&ovensCond, &ovensMutex);
  }

  availableOvens -= numPizzas;

  mutexUnlock(&ovensMutex);

  // Free cooks and signal other threads that wait for COOKS.
  mutexLock(&cooksMutex);

  availableCooks++;
  condBroadcast(&cooksCond);

  mutexUnlock(&cooksMutex);

  // Wait for pizzas to bake.
  sleep(T_BAKE);

  // Start time for cooling time of pizzas.
  startCoolTime = clock_gettime(CLOCK_REALTIME, &startCool);

  // 5. PACK -------------------------------------------------------------------
  // Wait for packeter.
  mutexLock(&packeterMutex);

  while(availablePacketers == 0){
    pthread_cond_wait(&packeterCond, &packeterMutex);
  }

  availablePacketers--;

  mutexUnlock(&packeterMutex);

  // Free ovens and signal other threads.
  mutexLock(&ovensMutex);

  availableOvens += numPizzas;
  condBroadcast(&ovensCond);

  mutexUnlock(&ovensMutex);

  sleep(T_PACK * numPizzas);

  int readyOrder = clock_gettime(CLOCK_REALTIME, &endPacket);

  mutexLock(&printMutex);
  while(printReady != 0){
    condWait(&printCond, &printMutex);
  }
  printReady = 1;
  mutexUnlock(&printMutex);

  printf("Order %d: Ready to deliver after %ld minutes.\n",*id, endPacket.tv_sec-startWait.tv_sec);

  mutexLock(&printMutex);
  printReady = 0;
  pthread_cond_broadcast(&printCond);
  mutexUnlock(&printMutex);

  // Free packeter and signal other threads.
  mutexLock(&packeterMutex);

  availablePacketers += 1;
  condBroadcast(&packeterCond);

  mutexUnlock(&packeterMutex);

  // 6. DELIVER ----------------------------------------------------------------
  // Wait for available deliverers.
  mutexLock(&deliverersMutex);

  while(availableDeliveres == 0){
    pthread_cond_wait(&delivererCond, &deliverersMutex);
  }
  availableDeliveres--;
  mutexUnlock(&deliverersMutex);

  deliveryTime = generateRandomNumber(T_DELIVER_LOW, T_DELIVER_HIGH, &seed);

  int endCoolTime = clock_gettime(CLOCK_REALTIME, &endOrder);
  coolTime = endOrder.tv_sec - startCool.tv_sec;

  // Update total cooling time.
  mutexLock(&coolTimeMutex);

  totalCoolTime += coolTime;
  if(maxCoolTime < coolTime){
    maxCoolTime = coolTime;
  }
  mutexUnlock(&coolTimeMutex);

  orderTime = endOrder.tv_sec- startWait.tv_sec;
  // Update total order time.
  mutexLock(&orderTimeMutex);

  totalOrderTime += orderTime;
  if(maxOrderTime < orderTime){
    maxOrderTime = orderTime;
  }
  mutexUnlock(&orderTimeMutex);

  sleep(deliveryTime);

  mutexLock(&printMutex);
  while(printReady != 0){
    pthread_cond_wait(&printCond, &printMutex);
  }
  printReady = 1;
  mutexUnlock(&printMutex);

  printf("Order %d: Order completed after %ld minutes.\n",*id, endOrder.tv_sec-startWait.tv_sec);

  mutexLock(&printMutex);
  printReady = 0;
  pthread_cond_broadcast(&printCond);
  mutexUnlock(&printMutex);

  sleep(deliveryTime);

  // Free deliverers.
  mutexLock(&deliverersMutex);

  availableDeliveres++;
  pthread_cond_broadcast(&delivererCond);
  mutexUnlock(&deliverersMutex);

  pthread_exit(0);
}

// Main thread
int main(int argc, char *argv[]){

  if(argc != 3){
    printf("ERROR: Invalid number of arguments.\n");
    exit(0);
  }

  // Assign command line arguments
  CUSTOMERS = atoi(argv[1]);
  seed = atoi(argv[2]);

  if(CUSTOMERS == 0){
    printf("No customers. Exiting...\n");
    exit(0);
  }

  // Threads
  pthread_t *threads = malloc((CUSTOMERS)*sizeof(pthread_t));
  if (threads == NULL){
    printf("ERROR: Failed to allocate memory for threads");
    exit(-1);
  }


  // Thread Ids
  int *threadId = malloc((CUSTOMERS)*sizeof(int));
  if (threads == NULL){
    printf("ERROR: Failed to allocate memory for thread ids");
    exit(-1);
  }

  // Initialize mutexes.
  pthread_mutex_init(&printMutex, NULL);
  pthread_mutex_init(&waitTimeMutex, NULL);
  pthread_mutex_init(&orderTimeMutex, NULL);
  pthread_mutex_init(&coolTimeMutex, NULL);
  pthread_mutex_init(&ordersFailMutex, NULL);
  pthread_mutex_init(&revenueMutex, NULL);
  pthread_mutex_init(&telsMutex, NULL);
  pthread_mutex_init(&cooksMutex, NULL);
  pthread_mutex_init(&ovensMutex, NULL);
  pthread_mutex_init(&packeterMutex, NULL);
  pthread_mutex_init(&deliverersMutex, NULL);

  // Create threads
  for (int i = 0; i < CUSTOMERS; i++){
    // Assign id.
    threadId[i] = i + 1;
    // Create thread.
    pthread_create(&threads[i], NULL, order, &threadId[i]);

    sleep(generateRandomNumber(T_ORDER_LOW, T_ORDER_HIGH, &seed));
  }

  // Join threads
  for(int i = 0; i < CUSTOMERS; i++){
    int status = pthread_join(threads[i], NULL);
    if(status != 0){
				printf("ERROR: Failed to join threads.\n");
		}
  }

  printf("--------------------\nPizzeria\n--------------------\n");
  printf("Revenue: %d\n",revenue);
  printf("Orders: Failed - %d, Succesful - %d\n", unsuccessfulOrders, CUSTOMERS-unsuccessfulOrders);
  printf("Order Time: Avg - %d, Max - %d\n",totalOrderTime/(CUSTOMERS-unsuccessfulOrders), maxOrderTime);
  printf("Wait Time: Avg - %d, Max - %d\n",totalWaitTime/(CUSTOMERS-unsuccessfulOrders), maxWaitTime);
  printf("Order Time: Avg - %d, Max - %d\n",totalCoolTime/(CUSTOMERS-unsuccessfulOrders), maxCoolTime);
  printf("--------------------\n");

  // Free memory
  free(threads);
  free(threadId);

  // Destroy mutexes.


  return 1;
}
