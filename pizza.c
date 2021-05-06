#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "pizza.h"

// Number threads to be created for each category.
int CUSTOMERS, TELS, COOKS, OVENS, DELIVERERS;

int PACKETER = 1;
// Min and max minutes it takes for a new customer to call.
int T_ORDER_LOW, T_ORDER_HIGH;
// Min and max minutes it takes for payment to be processed.
int T_PAYMENT_LOW, T_PAYMENT_HIGH;
// Min an max minutes it takes for pizzas to be delivered.
int T_DELIVER_LOW, T_DELIVER_HIGH;
// Min and max number of pizzas to be ordered.
int N_ORDER_LOW, N_ORDER_HIGH;
// Time it takes for cooks to prepare each pizza.
int T_PREP;
// Time it takes for a pizza to be baked.
int T_BAKE;
// Time it takes for packet guy to pack.
int T_PACK;

// Cost of pizza.
int COST;
// Probability payment fails.
int P;
// Total revenue at the end of the day.
int revenue = 0;

// Seed to create random numbers
int seed;

// Main thread
int main(int argc, char *argv[]){

  if(argc != 3){
    printf("ERROR: Invalid number of arguments.\n");
    exit(0);
  }


  // Assign command line arguments
  CUSTOMERS = atoi(argv[1]);
  seed = atoi(argv[2]);


  // Assign global variables
  readFile();

  // Threads
  pthread_t *threads = malloc((CUSTOMERS+TELS+COOKS+OVENS+DELIVERERS+PACKETER)*sizeof(pthread_t));
  if (threads == NULL){
    printf("ERROR: Failed to allocate memory for threads");
    exit(-1);
  }


  // Thread Ids
  int *threadId = malloc((CUSTOMERS+TELS+COOKS+OVENS+DELIVERERS+PACKETER)*sizeof(int));
  if (threads == NULL){
    printf("ERROR: Failed to allocate memory for thread ids");
    exit(-1);
  }

  // Create threads
  for(int i = 0; i < CUSTOMERS; i++){

    threadId[i] = i+1;
    printf("Thread %d - CUSTOMERS\n", threadId[i]);
    	// pthread_create(&threads[i], NULL, giveOrder, &threadIds[i]);
  }
  for(int i = CUSTOMERS; i < CUSTOMERS+TELS; i++ ){
    threadId[i] = i+1;
      printf("Thread %d - TELS\n", threadId[i]);
    	// pthread_create(&threads[i], NULL, receiveOrder, &threadIds[i]);
  }
  for(int i = CUSTOMERS+TELS; i < CUSTOMERS+TELS+COOKS; i++ ){
    threadId[i] = i+1;
      printf("Thread %d - COOKS\n", threadId[i]);
    	// pthread_create(&threads[i], NULL, cook, &threadIds[i]);
  }
  for(int i = CUSTOMERS+TELS+COOKS; i < CUSTOMERS+TELS+COOKS+OVENS; i++ ){
    threadId[i] = i+1;
      printf("Thread %d - OVENS\n", threadId[i]);
    	// pthread_create(&threads[i], NULL, bake, &threadIds[i]);
  }
  for(int i = CUSTOMERS+TELS+COOKS+OVENS; i < CUSTOMERS+TELS+COOKS+OVENS+PACKETER; i++ ){
    threadId[i] = i+1;
      printf("Thread %d - PACKETER\n", threadId[i]);
    	// pthread_create(&threads[i], NULL, pack, &threadIds[i]);
  }
  for(int i = CUSTOMERS+TELS+COOKS+OVENS+PACKETER; i < CUSTOMERS+TELS+COOKS+OVENS+PACKETER+DELIVERERS; i++ ){
    threadId[i] = i+1;
      printf("Thread %d - DELIVERERS\n", threadId[i]);
    	// pthread_create(&threads[i], NULL, deliver, &threadIds[i]);
  }

  // Join threads
  for(int i = 0; i < CUSTOMERS+TELS+COOKS+OVENS+PACKETER+DELIVERERS; i++){
    int status = pthread_join(threads[i], NULL);
    if(status != 0){
				printf("ERROR: Failed to join threads.\n");
		}
  }

  // Free memory
  free(threads);
  free(threadId);

  return 1;
}

// Function that reads file from arguments and assigns values.
void readFile(){

  char filename[15] = "constants.txt";
  FILE *file = fopen(filename, "r");

   if (file == NULL)
   {
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
   }

   // Name of variable to assign
   char name[20];
   // Value from file
   int value;

  // Assign global variables from file.
   while(fscanf(file, "%s = %d,", name, &value)>0){
     if (!strcmp(name,"TELS")){
       TELS = value;
     }else if(!strcmp(name,"COOKS")){
       COOKS = value;
     }else if(!strcmp(name, "OVENS")){
       OVENS = value;
     }else if(!strcmp(name, "DELIVERERS")){
       DELIVERERS = value;
     }else if(!strcmp(name, "T_ORDER_LOW")){
       T_ORDER_LOW = value;
     }else if(!strcmp(name, "T_ORDER_HIGH")){
       T_ORDER_HIGH = value;
     }else if(!strcmp(name, "N_ORDER_LOW")){
       N_ORDER_LOW = value;
     }else if(!strcmp(name, "N_ORDER_HIGH")){
       N_ORDER_HIGH = value;
     }else if(!strcmp( name, "T_PAYMENT_LOW")){
       T_PAYMENT_LOW = value;
     }else if(!strcmp(name, "T_PAYMENT_HIGH")){
       T_PAYMENT_HIGH = value;
     }else if(!strcmp(name, "COST")){
       COST = value;
     }else if(!strcmp(name, "P")){
       P = value;
     }else if(!strcmp(name, "T_PREP")){
       T_PREP = value;
     }else if (!strcmp(name, "T_BAKE")){
       T_BAKE = value;
     }else if(!strcmp(name,"T_PACK")){
       T_PACK = value;
     }else if(!strcmp(name,"T_DELIVER_LOW")){
       T_DELIVER_LOW = value;
     }else if (!strcmp(name, "T_DELIVER_HIGH")){
       T_DELIVER_HIGH = value;
     }
   }
   fclose(file);
}
