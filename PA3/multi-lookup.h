#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "queue.h"
#include "util.h"

#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

typedef struct input{
	FILE *file_name;
	pthread_mutex_t* qLock; // shared array lock
	pthread_mutex_t* sLock;	// service.txt lock
	FILE *service_name;
	queue *q;
	int num;
	
} inP;

typedef struct output{
	FILE *file_name;
	pthread_mutex_t* qLock; // array lock
	pthread_mutex_t* oLock;
	queue*q;
	int* request_exist;
} outP;

void* request(void* p);
void* resolve(void* p);

#endif
