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

void mutexLock(pthread_mutex_t *args){
  int rc = pthread_mutex_lock(args);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
}

void mutexUnlock(pthread_mutex_t *args){
  int rc = pthread_mutex_unlock(args);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }
}

int generateRandomNumber(int min, int max, unsigned int * seed){
  int randomNumber = rand_r(seed);
  // printf("Random number %d\n",randomNumber);
  randomNumber = (randomNumber%(max-min+1))+min;
  // printf("Random number after modulo %d\n", randomNumber);
  return randomNumber;
  // return rand_r(seed)%(max-min)+min;
}

void *order(void *args){
  // Cast void to int to get id.
  int *id = (int*)args;

  // Status.
  int rc;

  // Start time.
  struct timespec startWait, endWait, startCool, endPacket,endOrder;
  int orderTime;
  int waitTime;
  int coolTime;
  int deliveryTime;

  int startWaitTime = clock_gettime(CLOCK_REALTIME, &startWait);

  // Wait for available Tel.-----------------------------
  mutexLock(&telsMutex);

  while(availableTels == 0){
    pthread_cond_wait(&telsCond, &telsMutex);
  }

  availableTels--;
  mutexUnlock(&telsMutex);

  // Calculate wait time for this order.
  int endWaitTime = clock_gettime(CLOCK_REALTIME, &endWait);
  waitTime = endWait.tv_sec - startWait.tv_sec;

  // Add this waitTime to totalWaitTime.
  rc = pthread_mutex_lock(&waitTimeMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  totalWaitTime += waitTime;
  rc = pthread_mutex_unlock(&waitTimeMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  // Customer chooses a number of pizzas.
  int numPizzas = generateRandomNumber(N_ORDER_LOW, N_ORDER_HIGH, &seed);

  // Process payment.
  int totalPayment = COST * numPizzas;

  double pSuccess = (double)rand_r(&seed)/(double)RAND_MAX;

  sleep(generateRandomNumber(T_PAYMENT_LOW, T_PAYMENT_HIGH, &seed));

  if(pSuccess < P){
    rc = pthread_mutex_lock(&printMutex);
    if (rc != 0) {
      perror("ERROR: Mutex lock failed.\n");
      pthread_exit(&rc);
    }
    while(printReady != 0){
      pthread_cond_wait(&printCond, &printMutex);
    }
    printf("Order %d: Payment failed. Canceling...\n",*id);
    printReady = 0;
    rc = pthread_mutex_unlock(&printMutex);
    if (rc != 0) {
      perror("ERROR: Mutex unlock failed.\n");
      pthread_exit(&rc);
    }

    // Increase counter of failed orders.
    rc = pthread_mutex_lock(&telsMutex);
    if (rc != 0) {
      perror("ERROR: Mutex lock failed.\n");
      pthread_exit(&rc);
    }
    unsuccessfulOrders++;
    availableTels++;
    pthread_cond_broadcast(&telsCond);

    rc = pthread_mutex_unlock(&telsMutex);
    if (rc != 0) {
      perror("ERROR: Mutex unlock failed.\n");
      pthread_exit(&rc);
    }
    pthread_exit(0);
  }else{
    if (rc != 0) {
      perror("ERROR: Mutex lock failed.\n");
      pthread_exit(&rc);
    }
    while(printReady != 0){
      pthread_cond_wait(&printCond, &printMutex);
    }

    printf("Order %d: Payment OK.\n", *id);
    printReady = 0;
    pthread_cond_broadcast(&printCond);

    rc = pthread_mutex_unlock(&printMutex);
    if (rc != 0) {
      perror("ERROR: Mutex unlock failed.\n");
      pthread_exit(&rc);
    }

    // Increase total revenue.
    pthread_mutex_lock(&revenueMutex);
    revenue += totalPayment;
    rc = pthread_mutex_unlock(&revenueMutex);
    if (rc != 0) {
      perror("ERROR: Mutex unlock failed.\n");
      pthread_exit(&rc);
    }
  }

  // Free tels and sigmal other threads.
  pthread_mutex_lock(&telsMutex);

  availableTels++;
  rc = pthread_cond_broadcast(&telsCond);
  if (rc != 0) {
    perror("ERROR: Conditional broadcast failed.\n");
    pthread_exit(&rc);
  }

  pthread_mutex_unlock(&telsMutex);

  // Wait for available cook.
  rc = pthread_mutex_lock(&cooksMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }

  while(availableCooks == 0){
    pthread_cond_wait(&cooksCond, &cooksMutex);
  }

  availableCooks--;

  rc = pthread_mutex_unlock(&cooksMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  // Prepare pizzas.
  sleep(T_PREP * numPizzas);

  // Wait for available ovens.
  rc = pthread_mutex_lock(&ovensMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  while(availableOvens < numPizzas){
    pthread_cond_wait(&ovensCond, &ovensMutex);
  }

  availableOvens -= numPizzas;

  rc = pthread_mutex_unlock(&ovensMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  // Free cooks and signal other threads.
  rc = pthread_mutex_lock(&cooksMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  availableCooks++;
  rc = pthread_cond_broadcast(&cooksCond);
  if (rc != 0) {
    perror("ERROR: Conditional broadcast failed.\n");
    pthread_exit(&rc);
  }
  rc = pthread_mutex_unlock(&cooksMutex);

  // Wait for pizzas to bake.
  sleep(T_BAKE);

  // Start time for cooling time of pizzas.
  int startCoolTime = clock_gettime(CLOCK_REALTIME, &startCool);

  // Wait for packeter.
  rc = pthread_mutex_lock(&packeterMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  while(availablePacketers == 0){
    pthread_cond_wait(&packeterCond, &packeterMutex);
  }

  availablePacketers--;

  rc =  pthread_mutex_unlock(&packeterMutex);

  // Free ovens and signal other threads.
  rc = pthread_mutex_lock(&ovensMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  availableOvens += numPizzas;
  rc = pthread_cond_broadcast(&ovensCond);
  if (rc != 0) {
    perror("ERROR: Conditional broadcast failed.\n");
    pthread_exit(&rc);
  }
  rc = pthread_mutex_unlock(&ovensMutex);

  sleep(T_PACK * numPizzas);

  int readyOrder = clock_gettime(CLOCK_REALTIME, &endPacket);

  rc = pthread_mutex_lock(&printMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  while(printReady != 0){
    pthread_cond_wait(&printCond, &printMutex);
  }
  printf("Order %d: Ready to deliver after %ld minutes.\n",*id, endPacket.tv_sec-startWait.tv_sec);
  printReady = 0;
  pthread_cond_broadcast(&printCond);
  rc = pthread_mutex_unlock(&printMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  // Free packeter and signal other threads.
  rc = pthread_mutex_lock(&packeterMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  availablePacketers += 1;
  rc = pthread_cond_broadcast(&packeterCond);
  if (rc != 0) {
    perror("ERROR: Conditional broadcast failed.\n");
    pthread_exit(&rc);
  }
  rc = pthread_mutex_unlock(&packeterMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  // Wait for available deliverers.
  rc = pthread_mutex_lock(&deliverersMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  while(availableDeliveres == 0){
    pthread_cond_wait(&delivererCond, &deliverersMutex);
  }
  availableDeliveres--;
  rc = pthread_mutex_unlock(&deliverersMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }
  deliveryTime = generateRandomNumber(T_DELIVER_LOW, T_DELIVER_HIGH, &seed);

  int endCoolTime = clock_gettime(CLOCK_REALTIME, &endOrder);
  coolTime = endOrder.tv_sec - startCool.tv_sec;

  // Update total cooling time.
  rc = pthread_mutex_lock(&coolTimeMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  totalCoolTime += coolTime;
  if(maxCoolTime < coolTime){
    maxCoolTime = coolTime;
  }
  rc = pthread_mutex_unlock(&coolTimeMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  orderTime = endOrder.tv_sec- startWait.tv_sec;
  // Update total order time.
  rc = pthread_mutex_lock(&orderTimeMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  totalOrderTime += orderTime;
  if(maxOrderTime < orderTime){
    maxOrderTime = orderTime;
  }
  rc = pthread_mutex_unlock(&orderTimeMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

  sleep(deliveryTime);

  pthread_mutex_lock(&printMutex);
  while(printReady != 0){
    pthread_cond_wait(&printCond, &printMutex);
  }
  printf("Order %d: Order completed after %ld minutes.\n",*id, endOrder.tv_sec-startWait.tv_sec);
  printReady=0;
  pthread_cond_broadcast(&printCond);
  pthread_mutex_unlock(&printMutex);

  sleep(deliveryTime);

  // Free deliverers.
  rc = pthread_mutex_lock(&deliverersMutex);
  if (rc != 0) {
    perror("ERROR: Mutex lock failed.\n");
    pthread_exit(&rc);
  }
  availableDeliveres++;
  pthread_cond_broadcast(&delivererCond);
  rc = pthread_mutex_unlock(&deliverersMutex);
  if (rc != 0) {
    perror("ERROR: Mutex unlock failed.\n");
    pthread_exit(&rc);
  }

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

  return 1;
}
