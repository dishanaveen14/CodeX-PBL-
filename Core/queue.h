#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_CAPACITY 10

typedef struct {
    int items[QUEUE_CAPACITY];
    int front;
    int count;
} Queue;

void queue_init(Queue *queue);
int queue_enqueue(Queue *queue, int value);
int queue_dequeue(Queue *queue, int *value);
int queue_peek(const Queue *queue, int *value);
void queue_display(const Queue *queue);

#endif
