#include <stdio.h>
#ifndef QUEUE_H
#define QUEUE_H
typedef struct node_struct{
	void* load;
} queue_node;

typedef struct queue_struct{
	queue_node* array;
	int q_front;
	int q_rear;
	int q_size;
}queue;

int queue_i(queue *q, int size);
int qempty(queue *q);
int qfull(queue *q);
int q_push(queue* q, void* load);
void *q_pop(queue *q);
void q_clean(queue *q);

#endif
