#include "queue.h"
#include <stdlib.h>

int queue_i(queue*q, int size){
	if(size > 0){
		q->q_size = size;
	}else{
		printf("Please enter a reasonable queue size");
		return -1;
	}
	q->array = malloc(sizeof(queue_node) * size);
	if(!(q->array)){
		printf("oops, there's an error during the q allocation");
		return -1;
	}
	int j;
	for(j=0; j<size;++j){
		q->array[j].load = NULL;
	}
	q->q_front = 0;
	q->q_rear  = 0;
	return q->q_size;
}

int qempty(queue*q){
	if((q->q_front == q->q_rear) && (q->array[q->q_front].load == NULL)){
		return 1;
	}else{
		return 0;
	}
}

int qfull(queue *q){
	if((q->q_front == q->q_rear) && (q->array[q->q_front].load != NULL)){
		return 1;
	}else{
		return 0;
	}
}

int q_push(queue *q, void* input_load){
	if(qfull(q) == 1){
		printf("q is full brah");
		return -1;
	}
	q->array[q->q_rear].load = input_load;
	q->q_rear = q->q_rear+1;
	return 0;
}
void *q_pop(queue *q){
	void * popping;
	if(qempty(q)==1){
		return NULL;
	}
	popping = q->array[q->q_front].load;
	q->array[q->q_front].load = NULL;
	q->q_front = q->q_front +1;
	return popping;
}

void q_clean(queue *q){
	while(qempty(q)!=1){
		q_pop(q);
	}
	free(q->array);
}
