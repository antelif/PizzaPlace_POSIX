#include <stdio.h>

// Used by customer threads to order pizzas.
void *giveOrder(void *args);

// Used by tel threads to register orders and create payments.
void *receiveOrder(void *ars);

// Used by cook threads to prepare pizzas for given time.
void *cook(void *args);

// Used by ovens threads to bake pizzas for given time.
void *bake(void *args);

// Used by packer thread to pack available pizzas.
void *pack(void *args);

// Used by delivery threads to deliver pizzas.
void *deliver(void *args);
